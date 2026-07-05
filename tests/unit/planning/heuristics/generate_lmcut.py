#!/usr/bin/env python3
"""Regenerate lifted and ground LM-cut heuristic fixtures.

Computes hmax/hlmcut for the initial state and h* with A*+blind search.
A* uses a 1,000,000 state limit and a 10 second time limit for each cost mode.

Usage:
    .venv/bin/python tests/unit/planning/heuristics/generate_lmcut.py
    .venv/bin/python tests/unit/planning/heuristics/generate_lmcut.py Gripper TPP

Filtered runs write ``lmcut.json.generated`` next to the fixture instead of
replacing the full fixture.
"""

from __future__ import annotations

import json
import math
import os
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Literal, TypeAlias

from pypddl.formalism import ParserOptions
from pyyggdrasil.execution import ExecutionContext

from pytyr.formalism.planning import LiftedTask as FormalismLiftedTask
from pytyr.formalism.planning import Parser
from pytyr.planning import CostMode
from pytyr.planning import ground as ground_planning
from pytyr.planning import lifted as lifted_planning

ROOT = Path(__file__).resolve().parents[4]
MAX_NUM_STATES = 1_000_000
MAX_TIME = 10.0
HSTAR_TIMEOUT_SECONDS = 12
_KEEPALIVE: list[object] = []

TaskKind: TypeAlias = Literal["lifted", "ground"]
JsonNumber: TypeAlias = int | float
FixtureCase: TypeAlias = dict[str, object]

FIXTURES = {
    "lifted": ROOT / "tests/unit/planning/heuristics/lifted/lmcut.json",
    "ground": ROOT / "tests/unit/planning/heuristics/ground/lmcut.json",
}


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


def make_task(kind: TaskKind, domain_file: Path, task_file: Path) -> lifted_planning.Task | ground_planning.Task:
    lifted_task = lifted_planning.Task(parse_task(domain_file, task_file))
    _KEEPALIVE.append(lifted_task)
    if kind == "lifted":
        return lifted_task
    result = lifted_task.instantiate_ground_task(
        ExecutionContext(1),
        lifted_planning.GroundTaskInstantiationOptions(),
    )
    _KEEPALIVE.append(result)
    return result.task


def make_context(kind: TaskKind, domain_file: Path, task_file: Path) -> TaskContext:
    execution_context = ExecutionContext(1)
    if kind == "lifted":
        task = lifted_planning.Task(parse_task(domain_file, task_file))
        axiom_evaluator = lifted_planning.AxiomEvaluatorFactory().create(task, execution_context)
        state_repository = lifted_planning.StateRepositoryFactory().create(task, axiom_evaluator)
        successor_generator = lifted_planning.SuccessorGeneratorFactory().create(task, execution_context, state_repository)
        _KEEPALIVE.append((execution_context, task, axiom_evaluator, state_repository, successor_generator))
        return LiftedContext(execution_context, task, successor_generator)

    task = make_task("ground", domain_file, task_file)
    assert isinstance(task, ground_planning.Task)
    axiom_evaluator = ground_planning.AxiomEvaluatorFactory().create(task, execution_context)
    state_repository = ground_planning.StateRepositoryFactory().create(task, axiom_evaluator)
    successor_generator = ground_planning.SuccessorGeneratorFactory().create(task, execution_context, state_repository)
    _KEEPALIVE.append((execution_context, task, axiom_evaluator, state_repository, successor_generator))
    return GroundContext(execution_context, task, successor_generator)


def evaluate_initial(context: TaskContext, cost_mode: CostMode) -> tuple[JsonNumber, JsonNumber] | None:
    print(f"  evaluating {cost_mode} hmax/hlmcut", flush=True)
    state = context.successor_generator.get_initial_node().get_state()
    _KEEPALIVE.append(state)
    try:
        if isinstance(context, LiftedContext):
            hmax_heuristic = lifted_planning.MaxRPGHeuristic(context.task, context.execution_context, cost_mode)
            lmcut_heuristic = lifted_planning.LMCutHeuristic(context.task, context.execution_context, cost_mode)
        else:
            hmax_heuristic = ground_planning.MaxRPGHeuristic(context.task, context.execution_context, cost_mode)
            lmcut_heuristic = ground_planning.LMCutHeuristic(context.task, context.execution_context, cost_mode)
    except ValueError as error:
        print(f"Skipping {cost_mode} heuristic values: {error}", flush=True)
        return None
    _KEEPALIVE.extend((hmax_heuristic, lmcut_heuristic))
    print(f"    hmax start", flush=True)
    hmax = hmax_heuristic.evaluate(state)
    print(f"    hmax = {hmax}", flush=True)
    print(f"    hlmcut start", flush=True)
    hlmcut = lmcut_heuristic.evaluate(state)
    print(f"    hlmcut = {hlmcut}", flush=True)
    return as_json_number(float(hmax)), as_json_number(float(hlmcut))


def solve_hstar_direct(kind: TaskKind, domain_file: Path, task_file: Path, cost_mode: CostMode) -> JsonNumber | None:
    context = make_context(kind, domain_file, task_file)
    if isinstance(context, LiftedContext):
        heuristic = lifted_planning.BlindHeuristic()
        options = lifted_planning.astar_eager.Options()
        find_solution = lifted_planning.astar_eager.find_solution
    else:
        heuristic = ground_planning.BlindHeuristic()
        options = ground_planning.astar_eager.Options()
        find_solution = ground_planning.astar_eager.find_solution
    options.max_num_states = MAX_NUM_STATES
    options.max_time = MAX_TIME
    options.action_cost_mode = cost_mode
    result = find_solution(context.task, context.successor_generator, heuristic, options)
    plan = result.plan
    _KEEPALIVE.extend((heuristic, options, result, plan))
    if plan is None:
        return None
    return as_json_number(float(plan.get_cost()))


def solve_hstar(kind: TaskKind, domain_file: Path, task_file: Path, suffix: str, cost_mode: CostMode) -> JsonNumber | None:
    print(f"  solving {suffix} hstar", flush=True)
    command = [
        sys.executable,
        str(Path(__file__).resolve()),
        "--hstar-worker",
        kind,
        str(domain_file),
        str(task_file),
        suffix,
    ]
    try:
        result = subprocess.run(command, capture_output=True, text=True, timeout=HSTAR_TIMEOUT_SECONDS, check=False)
    except subprocess.TimeoutExpired:
        print(f"Skipping {kind} {suffix} hstar: external timeout after {HSTAR_TIMEOUT_SECONDS}s", flush=True)
        return None

    if result.returncode != 0:
        print(f"Skipping {kind} {suffix} hstar: worker failed with code {result.returncode}", flush=True)
        if result.stderr:
            print(result.stderr.strip(), flush=True)
        return None

    try:
        return json.loads(result.stdout.splitlines()[-1])["hstar"]
    except (IndexError, KeyError, json.JSONDecodeError) as error:
        print(f"Skipping {kind} {suffix} hstar: invalid worker output: {error}", flush=True)
        if result.stdout:
            print(result.stdout.strip(), flush=True)
        return None


def run_hstar_worker(argv: list[str]) -> None:
    _, kind, domain_file, task_file, suffix = argv
    if kind not in ("lifted", "ground"):
        raise ValueError(f"unknown task kind: {kind}")
    cost_mode = CostMode.UNIT if suffix == "unit" else CostMode.GENERAL
    hstar = solve_hstar_direct(kind, Path(domain_file), Path(task_file), cost_mode)
    print(json.dumps({"hstar": hstar}))
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(0)


def generate_case(kind: TaskKind, prefix: Path, case: FixtureCase) -> FixtureCase:
    domain_file = prefix / str(case["domain_file"])
    task_file = prefix / str(case["task_file"])
    context = make_context(kind, domain_file, task_file)

    result = {
        "name": case["name"],
        "domain_file": case["domain_file"],
        "task_file": case["task_file"],
    }

    supported_modes = []
    for suffix, cost_mode in (
        ("unit", CostMode.UNIT),
        ("general", CostMode.GENERAL),
    ):
        values = evaluate_initial(context, cost_mode)
        if values is None:
            continue
        hmax, hlmcut = values
        result[f"hmax_{suffix}"] = hmax
        result[f"hlmcut_{suffix}"] = hlmcut
        supported_modes.append((suffix, cost_mode))

    for suffix, cost_mode in supported_modes:
        hstar = solve_hstar(kind, domain_file, task_file, suffix, cost_mode)
        if hstar is not None:
            result[f"hstar_{suffix}"] = hstar

    return result


def generate_fixture(kind: TaskKind, fixture: Path, filters: set[str]) -> None:
    suite = json.loads(fixture.read_text())
    prefix = ROOT / suite["prefix"]
    cases = []
    for case in suite["cases"]:
        if filters and case["name"] not in filters:
            continue
        print(f"Generating {kind} :: {case['name']}", flush=True)
        cases.append(generate_case(kind, prefix, case))

    out = fixture if not filters else fixture.with_name(f"{fixture.name}.generated")
    out.write_text(json.dumps({"prefix": suite["prefix"], "cases": cases}, indent=4) + "\n")
    print(f"Wrote {out}")


def main() -> None:
    if len(sys.argv) > 1 and sys.argv[1] == "--hstar-worker":
        run_hstar_worker(sys.argv[1:])

    filters = set(sys.argv[1:])
    for kind, fixture in FIXTURES.items():
        generate_fixture(kind, fixture, filters)


if __name__ == "__main__":
    main()
    sys.stdout.flush()
    sys.stderr.flush()
    # ponytail: keep pytyr objects alive above and skip teardown; fixture generation only.
    os._exit(0)
