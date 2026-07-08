#!/usr/bin/env python3
"""Regenerate rpg_add heuristic fixtures."""

from __future__ import annotations

from pathlib import Path

from fixture_generation import ROOT, TaskKind, generate_main

FIXTURES: dict[TaskKind, Path] = {
    "ground": ROOT / "tests/unit/planning/heuristics/ground/rpg_add.json",
    "lifted": ROOT / "tests/unit/planning/heuristics/lifted/rpg_add.json",
}

if __name__ == "__main__":
    generate_main(Path(__file__).resolve(), FIXTURES, "rpg_add")
