#!/usr/bin/env python3
"""Regenerate optimal-cost fixtures with A* + blind search."""

from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path

from pytyr.planning import CostMode

from fixture_generation import COST_MODES, ROOT, TaskKind, as_json_number, make_context, planning_module

MAX_NUM_STATES = 1_000_000
MAX_TIME = 10.0
WORKER_TIMEOUT_SECONDS = 12

FIXTURES: dict[TaskKind, Path] = {
    "ground": ROOT / "tests/unit/planning/heuristics/ground/hstar.json",
    "lifted": ROOT / "tests/unit/planning/heuristics/lifted/hstar.json",
}


def solve_hstar(kind: TaskKind, domain_file: Path, task_file: Path, cost_mode: CostMode) -> int | float | None:
    context = make_context(kind, domain_file, task_file)
    planning = planning_module(context)
    heuristic = planning.BlindHeuristic()
    options = planning.astar_eager.Options()
    options.max_num_states = MAX_NUM_STATES
    options.max_time = MAX_TIME
    options.action_cost_mode = cost_mode
    result = planning.astar_eager.find_solution(context.task, context.successor_generator, heuristic, options)
    return None if result.plan is None else as_json_number(float(result.plan.get_cost()))


def run_worker(argv: list[str]) -> None:
    _, kind, domain_file, task_file, suffix = argv
    if kind not in ("ground", "lifted"):
        raise ValueError(f"unknown task kind: {kind}")
    cost_mode = CostMode.UNIT if suffix == "unit" else CostMode.GENERAL
    print(json.dumps({"h": solve_hstar(kind, Path(domain_file), Path(task_file), cost_mode)}))
    sys.stdout.flush()
    sys.stderr.flush()
    # ponytail: pytyr objects are process-local and this worker has produced its one JSON line.
    os._exit(0)


def solve_external(kind: TaskKind, domain_file: Path, task_file: Path, suffix: str) -> int | float | str:
    command = [sys.executable, str(Path(__file__).resolve()), "--worker", kind, str(domain_file), str(task_file), suffix]
    try:
        result = subprocess.run(command, capture_output=True, text=True, timeout=WORKER_TIMEOUT_SECONDS, check=False)
    except subprocess.TimeoutExpired:
        return f"worker timeout after {WORKER_TIMEOUT_SECONDS}s"

    if result.returncode != 0:
        if result.stderr:
            print(result.stderr.strip(), flush=True)
        return f"worker failed with code {result.returncode}"

    try:
        value = json.loads(result.stdout.splitlines()[-1])["h"]
    except (IndexError, KeyError, json.JSONDecodeError) as error:
        return f"invalid worker output: {error}"
    return "no plan within limits" if value is None else value


def generate_case(kind: TaskKind, prefix: Path, case: dict[str, object]) -> dict[str, object]:
    domain_file = prefix / str(case["domain_file"])
    task_file = prefix / str(case["task_file"])
    result: dict[str, object] = {"name": case["name"], "domain_file": case["domain_file"], "task_file": case["task_file"]}
    configs: dict[str, object] = {}
    skipped: dict[str, str] = {}

    for suffix, _ in COST_MODES:
        value = solve_external(kind, domain_file, task_file, suffix)
        if isinstance(value, str):
            skipped[suffix] = value
        else:
            configs[suffix] = {"h": value}

    if configs:
        result["configs"] = configs
    if skipped:
        result["skipped"] = skipped
    return result


def generate_fixture(kind: TaskKind, fixture: Path, filters: set[str]) -> None:
    suite = json.loads(fixture.read_text())
    prefix = ROOT / suite["prefix"]
    cases = []
    for case in suite["cases"]:
        if filters and case["name"] not in filters:
            continue
        print(f"Generating {kind}::hstar :: {case['name']}", flush=True)
        cases.append(generate_case(kind, prefix, case))

    out = fixture if not filters else fixture.with_name(f"{fixture.name}.generated")
    header = {key: value for key, value in suite.items() if key != "cases"}
    out.write_text(json.dumps({**header, "cases": cases}, indent=4) + "\n")
    print(f"Wrote {out}")


def main() -> None:
    if len(sys.argv) > 1 and sys.argv[1] == "--worker":
        run_worker(sys.argv[1:])

    filters = set(sys.argv[1:])
    for kind, fixture in FIXTURES.items():
        generate_fixture(kind, fixture, filters)

    sys.stdout.flush()
    sys.stderr.flush()
    # ponytail: fixture generation is a one-shot process; avoid pytyr teardown noise.
    os._exit(0)


if __name__ == "__main__":
    main()
