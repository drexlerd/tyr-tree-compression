#! /usr/bin/env python

import platform
import re
import sys

from pathlib import Path

import pypddl_datasets

from downward.reports.absolute import AbsoluteReport
from lab.environments import TetralithEnvironment, LocalEnvironment
from lab.experiment import Experiment
from lab.reports import Attribute, geometric_mean, arithmetic_mean

DIR = Path(__file__).resolve().parent
REPO = DIR.parent.parent.parent

sys.path.append(str(REPO))

from experiments.parser_search import SearchParser


# Create custom report class with suitable info and error attributes.
class BaseReport(AbsoluteReport):
    INFO_ATTRIBUTES = ["wall_time_limit", "memory_limit"]
    ERROR_ATTRIBUTES = [
        "domain",
        "problem",
        "algorithm",
        "unexplained_errors",
        "error",
        "node",
    ]



NODE = platform.node()
REMOTE = re.match(r"tetralith\d+.nsc.liu.se|n\d+", NODE)

NUM_THREADS = 1
RANDOM_SEED = 0

if REMOTE:
    ENV = TetralithEnvironment(
        setup=TetralithEnvironment.DEFAULT_SETUP,
        memory_per_cpu="8000M",
        cpus_per_task=1, 
        extra_options="#SBATCH --account=naiss2025-22-1245")
    # ENV.MAX_TASKS = 300
    
else:
    ENV = LocalEnvironment(processes=6)

if REMOTE:
    SUITES = [
        "ipc2023-numeric",
    ]
    WALL_TIME_LIMIT = 30 * 60
else:
    SUITES = [
        "ipc2023-numeric-test",
    ]
    WALL_TIME_LIMIT = 5

ATTRIBUTES = [
    "run_dir",
]
ATTRIBUTES += SearchParser.get_attributes()

MEMORY_LIMIT = 8000

# Create a new experiment.
exp = Experiment(environment=ENV)
exp.add_parser(SearchParser())

PLANNER_DIR = REPO / "build" / "exe" / "gbfs_lazy"

exp.add_resource("planner_exe", PLANNER_DIR)
exp.add_resource("run_planner", DIR.parent / "gbfs_lazy.sh")


for SUITE in SUITES:
    for domain in pypddl_datasets.fetch_suite(SUITE).domains:
        for heuristic in ["rpg_ff", "rpg_add", "blind", "goal_count"]:
            base_cmd = [
                "{run_planner}",
                "{planner_exe}",
                "{domain}",
                "{problem}",
                "plan.out",
                heuristic,
                str(NUM_THREADS),
                str(RANDOM_SEED),
            ]
        
        for task in domain.tasks:
                run = exp.add_run()
                run.add_resource("domain", task.domain_path, symlink=True)
                run.add_resource("problem", task.task_path, symlink=True)

                run.add_command(
                    f"gbfs-lazy-lifted-{heuristic}-{NUM_THREADS}",
                    base_cmd + ["-S"],
                    time_limit=None,
                    wall_time_limit=WALL_TIME_LIMIT,
                    memory_limit=MEMORY_LIMIT,
                )
                # AbsoluteReport needs the following properties:
                # 'domain', 'problem', 'algorithm', 'coverage'.
                run.set_property("domain", task.domain)
                run.set_property("problem", task.problem)
                run.set_property("algorithm", f"gbfs-lazy-lifted-{heuristic}-{NUM_THREADS}")
                # BaseReport needs the following properties:
                # 'time_limit', 'memory_limit'.
                run.set_property("wall_time_limit", WALL_TIME_LIMIT)
                run.set_property("memory_limit", MEMORY_LIMIT)
                # Every run has to have a unique id in the form of a list.
                # The algorithm name is only really needed when there are
                # multiple algorithms.
                run.set_property("id", [f"gbfs-lazy-lifted-{heuristic}-{NUM_THREADS}", task.domain, task.problem])

        for heuristic in ["blind", "goal_count"]:
            base_cmd = [
                "{run_planner}",
                "{planner_exe}",
                "{domain}",
                "{problem}",
                "plan.out",
                heuristic,
                str(NUM_THREADS),
                str(RANDOM_SEED),
            ]
        
        for task in domain.tasks:
                run = exp.add_run()
                run.add_resource("domain", task.domain_path, symlink=True)
                run.add_resource("problem", task.task_path, symlink=True)

                run.add_command(
                    f"gbfs-lazy-ground-{heuristic}-{NUM_THREADS}",
                    base_cmd + ["-S", "-G"],
                    time_limit=None,
                    wall_time_limit=WALL_TIME_LIMIT,
                    memory_limit=MEMORY_LIMIT,
                )
                # AbsoluteReport needs the following properties:
                # 'domain', 'problem', 'algorithm', 'coverage'.
                run.set_property("domain", task.domain)
                run.set_property("problem", task.problem)
                run.set_property("algorithm", f"gbfs-lazy-ground-{heuristic}-{NUM_THREADS}")
                # BaseReport needs the following properties:
                # 'time_limit', 'memory_limit'.
                run.set_property("wall_time_limit", WALL_TIME_LIMIT)
                run.set_property("memory_limit", MEMORY_LIMIT)
                # Every run has to have a unique id in the form of a list.
                # The algorithm name is only really needed when there are
                # multiple algorithms.
                run.set_property("id", [f"gbfs-lazy-ground-{heuristic}-{NUM_THREADS}", task.domain, task.problem])



# Add step that writes experiment files to disk.
exp.add_step("build", exp.build)

# Add step that executes all runs.
exp.add_step("start", exp.start_runs)

exp.add_step("parse", exp.parse)

# Add step that collects properties from run directories and
# writes them to *-eval/properties.
exp.add_fetcher(name="fetch")

# Make a report.
exp.add_report(BaseReport(attributes=ATTRIBUTES), outfile="report.html")

# Parse the commandline and run the specified steps.
exp.run_steps()