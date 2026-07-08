#!/usr/bin/env python3
"""Regenerate the iw search-statistics fixture (ground + lifted).

Each per-kind object records the search outcome (expected_status, expected_plan_length,
expected_solution_arity) plus the four counters aggregated across width iterations by the
iw event handler. The suite-level ``max_arity`` is preserved and used for the runs.

Usage:
    .venv/bin/python tests/unit/planning/algorithms/statistics/generate_iw.py [CASE ...]
"""

from __future__ import annotations

from pathlib import Path

from fixture_generation import (
    MAX_NUM_STATES,
    MAX_TIME,
    RECORDED_STATUSES,
    ROOT,
    SEARCH_STATUS_NAMES,
    _KEEPALIVE,
    ConfigSpec,
    ConfigResult,
    FixtureCase,
    CostSuffix,
    HeuristicName,
    TaskKind,
    counters_of,
    generate_main,
    make_context,
    planning_module,
)

FIXTURES: dict[TaskKind, Path] = {
    "ground": ROOT / "tests/unit/planning/algorithms/statistics/ground/iw.json",
    "lifted": ROOT / "tests/unit/planning/algorithms/statistics/lifted/iw.json",
}
CONFIGS: list[ConfigSpec] = [(None, None)]


def run_config(kind: TaskKind,
               heuristic_name: HeuristicName | None,
               cost_suffix: CostSuffix | None,
               domain_file: Path,
               task_file: Path,
               suite: FixtureCase) -> ConfigResult | str:
    max_arity = int(suite["max_arity"])
    context = make_context(kind, domain_file, task_file)
    planning = planning_module(context)

    brfs_solver = planning.brfs.Solver()
    brfs_solver.task = context.task
    brfs_solver.successor_generator = context.successor_generator
    brfs_solver.options.event_handler = planning.brfs.DefaultEventHandler()

    handler = planning.iw.DefaultEventHandler()
    options = planning.iw.Options()
    options.event_handler = handler
    options.max_num_states = MAX_NUM_STATES
    options.max_time = MAX_TIME

    result = planning.iw.find_solution(brfs_solver, max_arity, options)
    _KEEPALIVE.extend((brfs_solver, handler, options, result))

    if result.status not in RECORDED_STATUSES:
        return f"status {SEARCH_STATUS_NAMES[result.status]} within {MAX_TIME}s/{MAX_NUM_STATES} states"

    config: ConfigResult = {"expected_status": SEARCH_STATUS_NAMES[result.status]}
    plan = result.plan
    if plan is not None:
        _KEEPALIVE.append(plan)
        config["expected_plan_length"] = plan.get_length()
    solution_arity = handler.get_statistics().get_solution_arity()
    if solution_arity is not None:
        config["expected_solution_arity"] = solution_arity
    config.update(counters_of(handler.get_search_statistics()))
    return config


if __name__ == "__main__":
    generate_main(Path(__file__).resolve(), FIXTURES, CONFIGS, run_config)
