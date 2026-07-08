"""Shared plumbing for the per-algorithm search-statistics fixture generators.

Each ``generate_<algorithm>.py`` script defines its fixture path, configuration list, and
a ``run_config`` callable producing the per-configuration JSON object, then delegates to
:func:`generate_main`. Every configuration runs in a worker subprocess with an external
timeout; configurations that time out or fail are omitted from the fixture and skipped by
the tests. The recorded statistics pin the exact search trajectory, so the fixtures are
cross-platform determinism regression tests.

Filtered runs (case names as CLI arguments) write ``<fixture>.generated`` next to the
fixture instead of replacing it.
"""

from __future__ import annotations

import json
import math
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Literal, Mapping, NotRequired, Sequence, TypeAlias, TypedDict

from pypddl.formalism import ParserOptions
from pyyggdrasil.execution import ExecutionContext

from pytyr.formalism.planning import LiftedTask as FormalismLiftedTask
from pytyr.formalism.planning import Parser
from pytyr.planning import CostMode, SearchStatus, Statistics
from pytyr.planning import ground as ground_planning
from pytyr.planning import lifted as lifted_planning

ROOT = Path(__file__).resolve().parents[5]
MAX_NUM_STATES = 1_000_000
# Keep CI cheap: only configurations whose search finishes within MAX_TIME seconds are recorded;
# slower ones end OUT_OF_TIME and are flagged under the case-level "skipped" key so the gtest
# suites (and thus the GitHub action) never run them. The worker timeout additionally bounds
# parsing/grounding time.
MAX_TIME = 1.0
WORKER_TIMEOUT_SECONDS = 10
# Worker subprocesses are independent and single-threaded (ExecutionContext(1)), so running
# several concurrently only cuts regeneration wall time; recorded values are unaffected.
PARALLEL_WORKERS = 6
_KEEPALIVE: list[object] = []

TaskKind: TypeAlias = Literal["lifted", "ground"]
HeuristicName: TypeAlias = Literal["blind", "hmax", "hadd", "hff", "hlmcut"]
CostSuffix: TypeAlias = Literal["unit", "general"]
ConfigSpec: TypeAlias = tuple[HeuristicName | None, CostSuffix | None]
JsonNumber: TypeAlias = int | float
FixtureCase: TypeAlias = dict[str, object]
ConfigResult: TypeAlias = dict[str, object]
"""A run_config returns the per-configuration fixture object, or a skip reason string."""
RunConfig: TypeAlias = Callable[["TaskKind", "HeuristicName | None", "CostSuffix | None", Path, Path, "FixtureCase"], "ConfigResult | str"]

HEURISTICS: tuple[HeuristicName, ...] = ("blind", "hmax", "hadd", "hff", "hlmcut")
COST_SUFFIXES: tuple[CostSuffix, ...] = ("unit", "general")
RECORDED_STATUSES = (SearchStatus.SOLVED, SearchStatus.EXHAUSTED, SearchStatus.UNSOLVABLE)

SEARCH_STATUS_NAMES = {
    SearchStatus.SOLVED: "SOLVED",
    SearchStatus.EXHAUSTED: "EXHAUSTED",
    SearchStatus.UNSOLVABLE: "UNSOLVABLE",
    SearchStatus.CYCLE: "CYCLE",
    SearchStatus.FAILED: "FAILED",
    SearchStatus.IN_PROGRESS: "IN_PROGRESS",
    SearchStatus.OUT_OF_TIME: "OUT_OF_TIME",
    SearchStatus.OUT_OF_MEMORY: "OUT_OF_MEMORY",
    SearchStatus.OUT_OF_STATES: "OUT_OF_STATES",
}


class ConfigStatistics(TypedDict):
    num_generated: int
    num_expanded: int
    num_deadends: int
    num_pruned: int
    plan_length: NotRequired[int]
    plan_cost: NotRequired[JsonNumber]


@dataclass(frozen=True)
class LiftedContext:
    execution_context: ExecutionContext
    task: lifted_planning.Task
    successor_generator: lifted_planning.SuccessorGenerator


@dataclass(frozen=True)
class GroundContext:
    execution_context: ExecutionContext
    task: ground_planning.Task
    successor_generator: ground_planning.SuccessorGenerator


TaskContext: TypeAlias = LiftedContext | GroundContext


def as_json_number(value: float) -> JsonNumber:
    rounded = round(value)
    if math.isfinite(value) and abs(value - rounded) < 1e-9:
        return int(rounded)
    return value


def parse_task(domain_file: Path, task_file: Path) -> FormalismLiftedTask:
    parser_options = ParserOptions()
    parser_options.add_action_costs = True
    parser = Parser(str(domain_file), parser_options)
    task = parser.parse_task(str(task_file), parser_options)
    _KEEPALIVE.extend((parser_options, parser, task))
    return task


def planning_module(context: TaskContext):
    return lifted_planning if isinstance(context, LiftedContext) else ground_planning


def make_context(kind: TaskKind, domain_file: Path, task_file: Path) -> TaskContext:
    execution_context = ExecutionContext(1)
    lifted_task = lifted_planning.Task(parse_task(domain_file, task_file))
    _KEEPALIVE.append(lifted_task)

    if kind == "lifted":
        axiom_evaluator = lifted_planning.AxiomEvaluatorFactory().create(lifted_task, execution_context)
        state_repository = lifted_planning.StateRepositoryFactory().create(lifted_task, axiom_evaluator)
        successor_generator = lifted_planning.SuccessorGeneratorFactory().create(lifted_task, execution_context, state_repository)
        _KEEPALIVE.append((execution_context, axiom_evaluator, state_repository, successor_generator))
        return LiftedContext(execution_context, lifted_task, successor_generator)

    instantiation = lifted_task.instantiate_ground_task(ExecutionContext(1), lifted_planning.GroundTaskInstantiationOptions())
    task = instantiation.task
    axiom_evaluator = ground_planning.AxiomEvaluatorFactory().create(task, execution_context)
    state_repository = ground_planning.StateRepositoryFactory().create(task, axiom_evaluator)
    successor_generator = ground_planning.SuccessorGeneratorFactory().create(task, execution_context, state_repository)
    _KEEPALIVE.append((execution_context, instantiation, task, axiom_evaluator, state_repository, successor_generator))
    return GroundContext(execution_context, task, successor_generator)


def cost_mode_of(suffix: CostSuffix | None) -> CostMode:
    return CostMode.UNIT if suffix == "unit" else CostMode.GENERAL


def make_heuristic(context: TaskContext, name: HeuristicName, cost_suffix: CostSuffix | None) -> object:
    planning = planning_module(context)
    if name == "blind":
        return planning.BlindHeuristic()
    if name == "hmax":
        return planning.MaxRPGHeuristic(context.task, context.execution_context, cost_mode_of(cost_suffix))
    if name == "hadd":
        return planning.AddRPGHeuristic(context.task, context.execution_context, cost_mode_of(cost_suffix))
    if name == "hff":
        return planning.FFRPGHeuristic(context.task, context.execution_context, cost_mode_of(cost_suffix))
    if name == "hlmcut":
        return planning.LMCutHeuristic(context.task, context.execution_context, cost_mode_of(cost_suffix))
    raise ValueError(f"unknown heuristic: {name}")


def counters_of(statistics: Statistics) -> ConfigStatistics:
    return ConfigStatistics(
        num_generated=statistics.get_num_generated(),
        num_expanded=statistics.get_num_expanded(),
        num_deadends=statistics.get_num_deadends(),
        num_pruned=statistics.get_num_pruned(),
    )


def config_label(heuristic_name: HeuristicName | None, cost_suffix: CostSuffix | None) -> str:
    if heuristic_name is None:
        return "default"
    return f"{heuristic_name}_{cost_suffix}"


def search_module(context: TaskContext, algorithm: Literal["brfs", "astar_eager", "gbfs_lazy"]):
    planning = planning_module(context)
    if algorithm == "brfs":
        return planning.brfs
    if algorithm == "astar_eager":
        return planning.astar_eager
    if algorithm == "gbfs_lazy":
        return planning.gbfs_lazy
    raise ValueError(f"unknown algorithm: {algorithm}")


def run_search_config(
    algorithm: Literal["brfs", "astar_eager", "gbfs_lazy"],
    kind: TaskKind,
    heuristic_name: HeuristicName | None,
    cost_suffix: CostSuffix | None,
    domain_file: Path,
    task_file: Path,
    suite: FixtureCase,
) -> ConfigResult | str:
    """Run one plain-search configuration and return its fixture object (counters + plan)."""
    context = make_context(kind, domain_file, task_file)
    module = search_module(context, algorithm)
    handler = module.DefaultEventHandler()
    options = module.Options()
    options.event_handler = handler
    options.max_num_states = MAX_NUM_STATES
    options.max_time = MAX_TIME

    if heuristic_name is None:
        result = module.find_solution(context.task, context.successor_generator, options)
    else:
        options.action_cost_mode = cost_mode_of(cost_suffix)
        try:
            heuristic = make_heuristic(context, heuristic_name, cost_suffix)
        except ValueError as error:
            return f"unsupported heuristic: {error}"
        _KEEPALIVE.append(heuristic)
        result = module.find_solution(context.task, context.successor_generator, heuristic, options)
    _KEEPALIVE.extend((handler, options, result))

    if result.status not in RECORDED_STATUSES:
        return f"status {SEARCH_STATUS_NAMES[result.status]} within {MAX_TIME}s/{MAX_NUM_STATES} states"

    config: ConfigResult = dict(counters_of(handler.get_statistics()))
    plan = result.plan
    if plan is not None:
        _KEEPALIVE.append(plan)
        config["plan_length"] = plan.get_length()
        config["plan_cost"] = as_json_number(float(plan.get_cost()))
    return config


def run_config_external(
    script: Path,
    fixture: Path,
    case_name: str,
    kind: TaskKind,
    heuristic_name: HeuristicName | None,
    cost_suffix: CostSuffix | None,
    domain_file: Path,
    task_file: Path,
) -> ConfigResult | str:
    label = config_label(heuristic_name, cost_suffix)
    print(f"  running {kind}::{case_name} :: {label}", flush=True)
    command = [
        sys.executable,
        str(script),
        "--worker",
        str(fixture),
        kind,
        heuristic_name or "-",
        cost_suffix or "-",
        str(domain_file),
        str(task_file),
    ]
    try:
        result = subprocess.run(command, capture_output=True, text=True, timeout=WORKER_TIMEOUT_SECONDS, check=False)
    except subprocess.TimeoutExpired:
        return f"worker timeout after {WORKER_TIMEOUT_SECONDS}s"

    if result.returncode != 0:
        if result.stderr:
            print(result.stderr.strip(), flush=True)
        return f"worker failed with code {result.returncode}"

    try:
        payload = json.loads(result.stdout.splitlines()[-1])
    except (IndexError, json.JSONDecodeError) as error:
        return f"invalid worker output: {error}"
    return payload["config"] if payload.get("config") is not None else str(payload.get("reason", "unknown"))


def run_worker(run_config: RunConfig, argv: list[str]) -> None:
    _, fixture, kind, heuristic_name, cost_suffix, domain_file, task_file = argv
    if kind not in ("lifted", "ground"):
        raise ValueError(f"unknown task kind: {kind}")
    suite = json.loads(Path(fixture).read_text())
    config = run_config(kind, None if heuristic_name == "-" else heuristic_name, None if cost_suffix == "-" else cost_suffix, Path(domain_file), Path(task_file), suite)
    print(json.dumps({"config": None, "reason": config} if isinstance(config, str) else {"config": config}))
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(0)


def assemble_case(case: FixtureCase, configs: Sequence[ConfigSpec], results: dict[str, ConfigResult | str]) -> FixtureCase:
    result: FixtureCase = {
        "name": case["name"],
        "domain_file": case["domain_file"],
        "task_file": case["task_file"],
    }

    skipped: dict[str, str] = {}
    for heuristic_name, cost_suffix in configs:
        label = config_label(heuristic_name, cost_suffix)
        config = results[label]
        if isinstance(config, str):
            print(f"Skipping {case['name']} :: {label}: {config}", flush=True)
            skipped[label] = config
        else:
            result[label] = config
    if skipped:
        result["skipped"] = skipped

    return result


def generate_main(script: Path, fixtures: Mapping[TaskKind, Path], configs: Sequence[ConfigSpec], run_config: RunConfig) -> None:
    """Entry point for a generator script: handles --worker re-entry and case-name filters."""
    if len(sys.argv) > 1 and sys.argv[1] == "--worker":
        run_worker(run_config, sys.argv[1:])

    filters = set(sys.argv[1:])
    for kind, fixture in fixtures.items():
        suite = json.loads(fixture.read_text())
        prefix = ROOT / suite["prefix"]
        selected = [case for case in suite["cases"] if not filters or case["name"] in filters]

        with ThreadPoolExecutor(max_workers=PARALLEL_WORKERS) as pool:
            futures = {
                (str(case["name"]), config_label(heuristic_name, cost_suffix)):
                    pool.submit(run_config_external, script, fixture, str(case["name"]), kind, heuristic_name, cost_suffix,
                                prefix / str(case["domain_file"]), prefix / str(case["task_file"]))
                for case in selected
                for heuristic_name, cost_suffix in configs
            }
            results = {key: future.result() for key, future in futures.items()}

        cases = [
            assemble_case(case, configs, {label: result for (name, label), result in results.items() if name == case["name"]})
            for case in selected
        ]

        out = fixture if not filters else fixture.with_name(f"{fixture.name}.generated")
        suite_header = {key: value for key, value in suite.items() if key != "cases"}
        out.write_text(json.dumps({**suite_header, "cases": cases}, indent=4) + "\n")
        print(f"Wrote {out}")
