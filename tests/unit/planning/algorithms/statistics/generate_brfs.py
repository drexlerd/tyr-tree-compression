#!/usr/bin/env python3
"""Regenerate the brfs search-statistics fixture (heuristic-free, ground + lifted).

Usage:
    .venv/bin/python tests/unit/planning/algorithms/statistics/generate_brfs.py [CASE ...]
"""

from __future__ import annotations

from functools import partial
from pathlib import Path

from fixture_generation import ROOT, HeuristicName, TaskKind, generate_main, run_search_config

FIXTURE = ROOT / "tests/unit/planning/algorithms/statistics/brfs.json"
CONFIGS: list[tuple[TaskKind, HeuristicName | None]] = [("ground", None), ("lifted", None)]

if __name__ == "__main__":
    generate_main(Path(__file__).resolve(), FIXTURE, CONFIGS, partial(run_search_config, "brfs"))
