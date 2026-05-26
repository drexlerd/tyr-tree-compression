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

#include "tyr/common/json.hpp"
#include "tyr/common/json_suite.hpp"

#include <algorithm>
#include <deque>
#include <gtest/gtest.h>
#include <tyr/formalism/formalism.hpp>
#include <tyr/planning/planning.hpp>

namespace p = tyr::planning;
namespace fp = tyr::formalism::planning;

namespace tyr::tests
{
namespace
{
template<p::TaskKind Kind>
class SilentBrfsEventHandler : public p::brfs::EventHandler<Kind>
{
private:
    p::Statistics m_statistics;

public:
    void on_expand_node(const p::Node<Kind>& node) override { static_cast<void>(node); }
    void on_expand_goal_node(const p::Node<Kind>& node) override { static_cast<void>(node); }
    void on_generate_node(const p::Node<Kind>& source_node, const p::LabeledNode<Kind>& labeled_succ_node) override
    {
        static_cast<void>(source_node);
        static_cast<void>(labeled_succ_node);
    }
    void on_prune_node(const p::Node<Kind>& node) override { static_cast<void>(node); }
    void on_prune_node(const p::Node<Kind>& source_node, const p::LabeledNode<Kind>& labeled_succ_node) override
    {
        static_cast<void>(source_node);
        static_cast<void>(labeled_succ_node);
    }
    void on_start_search(const p::Node<Kind>& node) override
    {
        static_cast<void>(node);
        m_statistics = p::Statistics();
    }
    void on_finish_layer(uint_t layer) override { static_cast<void>(layer); }
    void on_end_search(p::SearchStatus status) override { static_cast<void>(status); }
    void on_solved(const p::Plan<Kind>& plan) override { static_cast<void>(plan); }
    const p::Statistics& get_search_statistics() const override { return m_statistics; }
    const p::Statistics& get_statistics() const override { return m_statistics; }
};

struct GroundSearchContext
{
    p::TaskPtr<p::GroundTag> task;
    p::SuccessorGeneratorPtr<p::GroundTag> successor_generator;
};

GroundSearchContext create_gripper_context()
{
    const auto root = tyr::common::root_path();
    const auto domain_file = root / "data/planning-benchmarks/tests/classical/gripper/domain.pddl";
    const auto task_file = root / "data/planning-benchmarks/tests/classical/gripper/test-1.pddl";

    auto execution_context = ExecutionContext::create(1);
    auto task = p::Task<p::LiftedTag>(fp::Parser(domain_file).parse_task(task_file)).instantiate_ground_task(*execution_context).task;
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::GroundTag>().create(task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::GroundTag>().create(task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::GroundTag>().create(task, execution_context, state_repository);

    return GroundSearchContext { std::move(task), std::move(successor_generator) };
}

class NeverSatisfiedGoalStrategy : public p::GoalStrategy<p::GroundTag>
{
public:
    bool is_static_goal_satisfied(const p::Task<p::GroundTag>& task) override
    {
        static_cast<void>(task);
        return true;
    }

    bool is_dynamic_goal_satisfied(const p::StateView<p::GroundTag>& seed_state, const p::StateView<p::GroundTag>& state) override
    {
        static_cast<void>(seed_state);
        static_cast<void>(state);
        return false;
    }
};

class ScriptedSolver
{
private:
    std::shared_ptr<std::deque<p::SearchResult<p::GroundTag>>> m_results;

public:
    using EventHandlerType = p::brfs::EventHandler<p::GroundTag>;

    p::brfs::Options<p::GroundTag> options;

    explicit ScriptedSolver(std::deque<p::SearchResult<p::GroundTag>> results) :
        m_results(std::make_shared<std::deque<p::SearchResult<p::GroundTag>>>(std::move(results)))
    {
        options.event_handler = std::make_shared<SilentBrfsEventHandler<p::GroundTag>>();
    }

    p::SearchResult<p::GroundTag> solve()
    {
        auto result = std::move(m_results->front());
        m_results->pop_front();
        return result;
    }
};
}

TEST(TyrPlanningSerialized, BrfsSubsolverMatchesDirectBrfs)
{
    auto direct_context = create_gripper_context();
    auto serialized_context = create_gripper_context();

    auto direct_options = p::brfs::Options<p::GroundTag> {};
    direct_options.event_handler = std::make_shared<SilentBrfsEventHandler<p::GroundTag>>();
    const auto direct_result = p::brfs::find_solution(*direct_context.task, *direct_context.successor_generator, direct_options);

    auto brfs_solver = p::brfs::Solver<p::GroundTag> { serialized_context.task, serialized_context.successor_generator, p::brfs::Options<p::GroundTag> {} };
    brfs_solver.options.event_handler = std::make_shared<SilentBrfsEventHandler<p::GroundTag>>();

    auto serialized_options = p::serialized::Options<p::GroundTag, decltype(brfs_solver)> {};
    const auto event_handler = p::serialized::DefaultEventHandler<p::GroundTag, decltype(brfs_solver)>::create();
    serialized_options.event_handler = event_handler;
    serialized_options.subgoal_strategy = p::SerializedGoalStrategy<p::GroundTag>::create(*serialized_context.task);
    serialized_options.goal_strategy = p::ConjunctiveGoalStrategy<p::GroundTag>::create(*serialized_context.task);

    const auto serialized_result = p::serialized::find_solution(brfs_solver, serialized_options);

    ASSERT_EQ(direct_result.status, p::SearchStatus::SOLVED);
    ASSERT_TRUE(direct_result.plan);
    ASSERT_EQ(serialized_result.status, p::SearchStatus::SOLVED);
    ASSERT_TRUE(serialized_result.plan);

    EXPECT_EQ(serialized_result.plan->get_length(), direct_result.plan->get_length());
    EXPECT_GE(event_handler->get_statistics().get_num_subsearches(), 1);
    EXPECT_EQ(event_handler->get_statistics().get_search_statistics().size(), event_handler->get_statistics().get_num_subsearches());
    EXPECT_EQ(event_handler->get_statistics().get_solver_statistics().size(), event_handler->get_statistics().get_num_subsearches());
}

TEST(TyrPlanningSerialized, DetectsRepeatedSubgoalState)
{
    auto context = create_gripper_context();
    auto start_node = context.successor_generator->get_initial_node();
    auto labeled_succ_nodes = p::LabeledNodeList<p::GroundTag> {};
    context.successor_generator->get_labeled_successor_nodes(start_node, labeled_succ_nodes);
    ASSERT_FALSE(labeled_succ_nodes.empty());

    const auto first_labeled_succ_node_it =
        std::find_if(labeled_succ_nodes.begin(),
                     labeled_succ_nodes.end(),
                     [&](const auto& labeled_succ_node)
                     { return labeled_succ_node.node.get_state().get_index() != start_node.get_state().get_index(); });
    ASSERT_NE(first_labeled_succ_node_it, labeled_succ_nodes.end());
    const auto first_labeled_succ_node = *first_labeled_succ_node_it;
    const auto first_goal_node = first_labeled_succ_node.node;
    const auto second_start_node = p::Node<p::GroundTag>(first_goal_node.get_state(), 0);
    const auto repeated_start_node = p::Node<p::GroundTag>(start_node.get_state(), 1);

    auto first_subresult = p::SearchResult<p::GroundTag> {};
    first_subresult.status = p::SearchStatus::SOLVED;
    first_subresult.goal_node = first_goal_node;
    first_subresult.plan = p::Plan<p::GroundTag>(start_node, p::LabeledNodeList<p::GroundTag> { first_labeled_succ_node });

    auto second_subresult = p::SearchResult<p::GroundTag> {};
    second_subresult.status = p::SearchStatus::SOLVED;
    second_subresult.goal_node = repeated_start_node;
    second_subresult.plan = p::Plan<p::GroundTag>(second_start_node,
                                                  p::LabeledNodeList<p::GroundTag> { p::LabeledNode<p::GroundTag> {
                                                      first_labeled_succ_node.label,
                                                      repeated_start_node,
                                                  } });

    auto solver = ScriptedSolver(std::deque<p::SearchResult<p::GroundTag>> { std::move(first_subresult), std::move(second_subresult) });

    auto options = p::serialized::Options<p::GroundTag, ScriptedSolver> {};
    options.event_handler = p::serialized::DefaultEventHandler<p::GroundTag, ScriptedSolver>::create();
    options.subgoal_strategy = std::make_shared<NeverSatisfiedGoalStrategy>();
    options.goal_strategy = std::make_shared<NeverSatisfiedGoalStrategy>();

    const auto result = p::serialized::find_solution(solver, options);

    ASSERT_EQ(result.status, p::SearchStatus::CYCLE);
    ASSERT_TRUE(result.plan);
    ASSERT_TRUE(result.cycle_range);
    EXPECT_EQ(result.plan->get_length(), 2);
    EXPECT_EQ(result.cycle_range->first, 0);
    EXPECT_EQ(result.cycle_range->second, 2);
}

}
