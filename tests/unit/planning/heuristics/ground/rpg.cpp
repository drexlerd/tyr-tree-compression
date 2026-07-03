/*
 * Copyright (C) 2025-2026 Dominik Drexler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "tyr/formalism/formalism.hpp"
#include "tyr/planning/planning.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <limits>

namespace fp = tyr::formalism::planning;
namespace p = tyr::planning;

namespace tyr::tests
{

TEST(TyrPlanningGroundRPGHeuristics, InitialStateIsFiniteOnBlocks3)
{
    const auto root = std::filesystem::path(ROOT_DIR);
    auto lifted_task = p::Task<p::LiftedTag>(fp::Parser(root / "data/planning-benchmarks/tests/classical/blocks_3/domain.pddl")
                                                 .parse_task(root / "data/planning-benchmarks/tests/classical/blocks_3/test-1.pddl"));
    auto grounding_context = ygg::ExecutionContext(1);
    auto ground_task = lifted_task.instantiate_ground_task(grounding_context).task;

    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::GroundTag>().create(ground_task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::GroundTag>().create(ground_task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::GroundTag>().create(ground_task, execution_context, state_repository);
    const auto initial_state = successor_generator->get_initial_node().get_state();

    auto hmax = p::MaxRPGHeuristic<p::GroundTag>::create(ground_task, execution_context);
    auto hadd = p::AddRPGHeuristic<p::GroundTag>::create(ground_task, execution_context);
    auto hff = p::FFRPGHeuristic<p::GroundTag>::create(ground_task, execution_context);
    auto lmcut = p::LMCutHeuristic<p::GroundTag>::create(ground_task, execution_context);

    const auto hmax_value = hmax->evaluate(initial_state);
    const auto hadd_value = hadd->evaluate(initial_state);
    const auto hff_value = hff->evaluate(initial_state);
    const auto lmcut_value = lmcut->evaluate(initial_state);

    EXPECT_NE(hmax_value, std::numeric_limits<ygg::float_t>::infinity());
    EXPECT_NE(hadd_value, std::numeric_limits<ygg::float_t>::infinity());
    EXPECT_NE(hff_value, std::numeric_limits<ygg::float_t>::infinity());
    EXPECT_NE(lmcut_value, std::numeric_limits<ygg::float_t>::infinity());
    EXPECT_GE(hadd_value, hmax_value);
    EXPECT_GE(lmcut_value, hmax_value);
}

}
