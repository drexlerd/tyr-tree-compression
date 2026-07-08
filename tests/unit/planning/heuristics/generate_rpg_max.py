#!/usr/bin/env python3
"""Regenerate rpg_max heuristic fixtures."""

from __future__ import annotations

from pathlib import Path

from fixture_generation import ROOT, TaskKind, generate_main

FIXTURES: dict[TaskKind, Path] = {
    "ground": ROOT / "tests/unit/planning/heuristics/ground/rpg_max.json",
    "lifted": ROOT / "tests/unit/planning/heuristics/lifted/rpg_max.json",
}

if __name__ == "__main__":
    generate_main(Path(__file__).resolve(), FIXTURES, "rpg_max")
