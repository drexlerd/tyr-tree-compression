#!/usr/bin/env python3
"""Regenerate the gbfs (lazy) search-statistics fixture ({ground, lifted} x all heuristics).

Usage:
    .venv/bin/python tests/unit/planning/algorithms/statistics/generate_gbfs.py [CASE ...]
"""

from __future__ import annotations

from functools import partial
from pathlib import Path

from fixture_generation import HEURISTICS, ROOT, HeuristicName, TaskKind, generate_main, run_search_config

FIXTURE = ROOT / "tests/unit/planning/algorithms/statistics/gbfs.json"
CONFIGS: list[tuple[TaskKind, HeuristicName | None]] = [(kind, heuristic) for kind in ("ground", "lifted") for heuristic in HEURISTICS]

if __name__ == "__main__":
    generate_main(Path(__file__).resolve(), FIXTURE, CONFIGS, partial(run_search_config, "gbfs_lazy"))
