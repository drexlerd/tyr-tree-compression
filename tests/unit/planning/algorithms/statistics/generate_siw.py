#!/usr/bin/env python3
"""Regenerate the siw search-statistics fixture (ground + lifted).

Each per-kind object records the search outcome (expected_status,
expected_maximum_effective_width) plus the four counters aggregated across subsearches by
the siw event handler. The suite-level ``max_arity`` is preserved and used for the runs.

Usage:
    .venv/bin/python tests/unit/planning/algorithms/statistics/generate_siw.py [CASE ...]
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
    ConfigResult,
    FixtureCase,
    HeuristicName,
    TaskKind,
    counters_of,
    generate_main,
    make_context,
    planning_module,
)

FIXTURE = ROOT / "tests/unit/planning/algorithms/statistics/siw.json"
CONFIGS: list[tuple[TaskKind, HeuristicName | None]] = [("ground", None), ("lifted", None)]


def run_config(kind: TaskKind, heuristic_name: HeuristicName | None, domain_file: Path, task_file: Path, suite: FixtureCase) -> ConfigResult | str:
    max_arity = int(suite["max_arity"])
    context = make_context(kind, domain_file, task_file)
    planning = planning_module(context)

    brfs_solver = planning.brfs.Solver()
    brfs_solver.task = context.task
    brfs_solver.successor_generator = context.successor_generator
    brfs_solver.options.event_handler = planning.brfs.DefaultEventHandler()
    brfs_solver.options.max_num_states = MAX_NUM_STATES
    brfs_solver.options.max_time = MAX_TIME

    iw_solver = planning.iw.Solver()
    iw_solver.brfs_solver = brfs_solver
    iw_solver.max_arity = max_arity

    handler = planning.siw.DefaultEventHandler()
    options = planning.siw.Options()
    options.event_handler = handler

    result = planning.siw.find_solution(iw_solver, options)
    _KEEPALIVE.extend((brfs_solver, iw_solver, handler, options, result))

    if result.status not in RECORDED_STATUSES:
        return f"status {SEARCH_STATUS_NAMES[result.status]} within {MAX_TIME}s/{MAX_NUM_STATES} states"

    config: ConfigResult = {"expected_status": SEARCH_STATUS_NAMES[result.status]}
    maximum_effective_width = handler.get_statistics().get_maximum_effective_width()
    if maximum_effective_width is not None:
        config["expected_maximum_effective_width"] = maximum_effective_width
    config.update(counters_of(handler.get_search_statistics()))
    return config


if __name__ == "__main__":
    generate_main(Path(__file__).resolve(), FIXTURE, CONFIGS, run_config)
