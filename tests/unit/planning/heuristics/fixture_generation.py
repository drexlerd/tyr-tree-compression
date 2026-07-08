"""Shared plumbing for heuristic fixture generators."""

from __future__ import annotations

import json
import math
import os
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
_KEEPALIVE: list[object] = []

TaskKind: TypeAlias = Literal["lifted", "ground"]
HeuristicName: TypeAlias = Literal["rpg_max", "rpg_add", "rpg_ff", "lmcut"]
JsonNumber: TypeAlias = int | float
FixtureCase: TypeAlias = dict[str, object]
TaskContext: TypeAlias = "LiftedContext | GroundContext"

COST_MODES: tuple[tuple[str, CostMode], ...] = (("unit", CostMode.UNIT), ("general", CostMode.GENERAL))


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


def make_context(kind: TaskKind, domain_file: Path, task_file: Path) -> TaskContext:
    execution_context = ExecutionContext(1)
    lifted_task = lifted_planning.Task(parse_task(domain_file, task_file))
    _KEEPALIVE.append(lifted_task)

    if kind == "lifted":
        axiom_evaluator = lifted_planning.AxiomEvaluatorFactory().create(lifted_task, execution_context)
        state_repository = lifted_planning.StateRepositoryFactory().create(lifted_task, axiom_evaluator)
        successor_generator = lifted_planning.SuccessorGeneratorFactory().create(lifted_task, execution_context, state_repository)
        _KEEPALIVE.extend((execution_context, axiom_evaluator, state_repository, successor_generator))
        return LiftedContext(execution_context, lifted_task, successor_generator)

    instantiation = lifted_task.instantiate_ground_task(ExecutionContext(1), lifted_planning.GroundTaskInstantiationOptions())
    task = instantiation.task
    axiom_evaluator = ground_planning.AxiomEvaluatorFactory().create(task, execution_context)
    state_repository = ground_planning.StateRepositoryFactory().create(task, axiom_evaluator)
    successor_generator = ground_planning.SuccessorGeneratorFactory().create(task, execution_context, state_repository)
    _KEEPALIVE.extend((execution_context, instantiation, task, axiom_evaluator, state_repository, successor_generator))
    return GroundContext(execution_context, task, successor_generator)


def planning_module(context: TaskContext):
    return lifted_planning if isinstance(context, LiftedContext) else ground_planning


def make_heuristic(context: TaskContext, heuristic_name: HeuristicName, cost_mode: CostMode) -> object:
    planning = planning_module(context)
    if heuristic_name == "rpg_max":
        return planning.MaxRPGHeuristic(context.task, context.execution_context, cost_mode)
    if heuristic_name == "rpg_add":
        return planning.AddRPGHeuristic(context.task, context.execution_context, cost_mode)
    if heuristic_name == "rpg_ff":
        return planning.FFRPGHeuristic(context.task, context.execution_context, cost_mode)
    if heuristic_name == "lmcut":
        return planning.LMCutHeuristic(context.task, context.execution_context, cost_mode)
    raise ValueError(f"unknown heuristic: {heuristic_name}")


def evaluate_initial(context: TaskContext, heuristic_name: HeuristicName, cost_mode: CostMode) -> JsonNumber:
    state = context.successor_generator.get_initial_node().get_state()
    heuristic = make_heuristic(context, heuristic_name, cost_mode)
    _KEEPALIVE.extend((state, heuristic))
    return as_json_number(float(heuristic.evaluate(state)))


def generate_case(kind: TaskKind, heuristic_name: HeuristicName, prefix: Path, case: FixtureCase) -> FixtureCase:
    domain_file = prefix / str(case["domain_file"])
    task_file = prefix / str(case["task_file"])
    context = make_context(kind, domain_file, task_file)
    result: FixtureCase = {"name": case["name"], "domain_file": case["domain_file"], "task_file": case["task_file"]}
    configs: dict[str, object] = {}
    skipped: dict[str, str] = {}

    for suffix, cost_mode in COST_MODES:
        try:
            configs[suffix] = {"h": evaluate_initial(context, heuristic_name, cost_mode)}
        except ValueError as error:
            skipped[suffix] = str(error)

    if configs:
        result["configs"] = configs
    if skipped:
        result["skipped"] = skipped
    return result


def generate_fixture(script: Path, kind: TaskKind, fixture: Path, heuristic_name: HeuristicName, filters: set[str]) -> None:
    suite = json.loads(fixture.read_text())
    prefix = ROOT / suite["prefix"]
    cases = []
    for case in suite["cases"]:
        if filters and case["name"] not in filters:
            continue
        print(f"Generating {kind}::{heuristic_name} :: {case['name']}", flush=True)
        cases.append(generate_case(kind, heuristic_name, prefix, case))

    out = fixture if not filters else fixture.with_name(f"{fixture.name}.generated")
    header = {key: value for key, value in suite.items() if key != "cases"}
    out.write_text(json.dumps({**header, "cases": cases}, indent=4) + "\n")
    print(f"Wrote {out}")


def generate_main(script: Path, fixtures: dict[TaskKind, Path], heuristic_name: HeuristicName) -> None:
    filters = set(sys.argv[1:])
    for kind, fixture in fixtures.items():
        generate_fixture(script, kind, fixture, heuristic_name, filters)

    sys.stdout.flush()
    sys.stderr.flush()
    # ponytail: pytyr teardown can be noisy; these scripts are one-shot fixture generators.
    os._exit(0)
