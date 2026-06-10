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

#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <tyr/formalism/formalism.hpp>
#include <tyr/planning/planning.hpp>
#include <vector>
#include <yggdrasil/serialization/json.hpp>
#include <yggdrasil/serialization/json_suite.hpp>

namespace p = tyr::planning;
namespace f = tyr::formalism;
namespace fp = tyr::formalism::planning;

namespace tyr::tests
{
namespace
{
struct GroundTaskCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
    size_t expected_fluent_atoms;
    size_t expected_derived_atoms;
    size_t expected_actions;
    size_t expected_axioms;
    size_t expected_successors;
};

GroundTaskCase parse_case(const boost::json::object& suite, const boost::json::object& object)
{
    return GroundTaskCase { ygg::common::as_string(object, "name", "case"),
                            ygg::common::suite_path(suite, ygg::common::as_string(object, "domain_file", "case")),
                            ygg::common::suite_path(suite, ygg::common::as_string(object, "task_file", "case")),
                            ygg::common::as_size(object, "expected_fluent_atoms", "case"),
                            ygg::common::as_size(object, "expected_derived_atoms", "case"),
                            ygg::common::as_size(object, "expected_actions", "case"),
                            ygg::common::as_size(object, "expected_axioms", "case"),
                            ygg::common::as_size(object, "expected_successors", "case") };
}

std::vector<GroundTaskCase> load_cases()
{
    const auto suite = ygg::common::load_json_file(ygg::common::root_path() / "tests/unit/planning/ground_task.json");
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    auto result = std::vector<GroundTaskCase> {};
    for (const auto& case_value : ygg::common::as_array(suite_object, "cases", "suite"))
        result.push_back(parse_case(suite_object, ygg::common::as_object(case_value, "case")));
    return result;
}

}

class GroundTaskTest : public ::testing::TestWithParam<GroundTaskCase>
{
};

TEST_P(GroundTaskTest, StateViewsUseRepositoryContextForIdentity)
{
    const auto& param = GetParam();
    auto execution_context = ygg::ExecutionContext(1);
    auto ground_task = p::Task<p::LiftedTag>(fp::Parser(param.domain_file).parse_task(param.task_file)).instantiate_ground_task(execution_context).task;

    auto shared_execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator_factory = p::AxiomEvaluatorFactory<p::GroundTag>();
    auto state_repository_factory = p::StateRepositoryFactory<p::GroundTag>();
    auto successor_generator_factory = p::SuccessorGeneratorFactory<p::GroundTag>();

    auto first_axiom_evaluator = axiom_evaluator_factory.create(ground_task, shared_execution_context);
    auto second_axiom_evaluator = axiom_evaluator_factory.create(ground_task, shared_execution_context);

    auto first_repository = state_repository_factory.create(ground_task, first_axiom_evaluator);
    auto second_repository = state_repository_factory.create(ground_task, second_axiom_evaluator);

    auto first_successor_generator = successor_generator_factory.create(ground_task, shared_execution_context, first_repository);
    auto second_successor_generator = successor_generator_factory.create(ground_task, shared_execution_context, second_repository);

    const auto first_state = first_repository->get_initial_state();
    const auto second_state = second_repository->get_initial_state();

    EXPECT_EQ(first_axiom_evaluator->get_index(), 0);
    EXPECT_EQ(second_axiom_evaluator->get_index(), 1);
    EXPECT_EQ(first_repository->get_index(), 0);
    EXPECT_EQ(second_repository->get_index(), 1);
    EXPECT_EQ(first_successor_generator->get_index(), 0);
    EXPECT_EQ(second_successor_generator->get_index(), 1);
    EXPECT_EQ(first_successor_generator->get_state_repository(), first_repository);
    EXPECT_EQ(second_successor_generator->get_state_repository(), second_repository);
    EXPECT_EQ(first_state.get_index(), second_state.get_index());
    EXPECT_FALSE(ygg::EqualTo<p::StateView<p::GroundTag>> {}(first_state, second_state));
    EXPECT_NE(ygg::Hash<p::StateView<p::GroundTag>> {}(first_state), ygg::Hash<p::StateView<p::GroundTag>> {}(second_state));
}

TEST_P(GroundTaskTest, StateViewsFromIndependentFactoriesUseDeterministicFactoryLocalIdentity)
{
    const auto& param = GetParam();
    auto execution_context = ygg::ExecutionContext(1);
    auto ground_task = p::Task<p::LiftedTag>(fp::Parser(param.domain_file).parse_task(param.task_file)).instantiate_ground_task(execution_context).task;

    auto shared_execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator_factory = p::AxiomEvaluatorFactory<p::GroundTag>();
    auto first_state_repository_factory = p::StateRepositoryFactory<p::GroundTag>();
    auto second_state_repository_factory = p::StateRepositoryFactory<p::GroundTag>();

    auto first_axiom_evaluator = axiom_evaluator_factory.create(ground_task, shared_execution_context);
    auto second_axiom_evaluator = axiom_evaluator_factory.create(ground_task, shared_execution_context);

    auto first_repository = first_state_repository_factory.create(ground_task, first_axiom_evaluator);
    auto second_repository = second_state_repository_factory.create(ground_task, second_axiom_evaluator);

    const auto first_state = first_repository->get_initial_state();
    const auto second_state = second_repository->get_initial_state();

    EXPECT_EQ(first_repository->get_index(), 0);
    EXPECT_EQ(second_repository->get_index(), 0);
    EXPECT_EQ(first_state.get_index(), second_state.get_index());
    EXPECT_TRUE(ygg::EqualTo<p::StateView<p::GroundTag>> {}(first_state, second_state));
    EXPECT_EQ(ygg::Hash<p::StateView<p::GroundTag>> {}(first_state), ygg::Hash<p::StateView<p::GroundTag>> {}(second_state));
}

TEST_P(GroundTaskTest, CreateStateOverloadsCanonicalizeToSameRegisteredState)
{
    const auto& param = GetParam();
    auto execution_context = ygg::ExecutionContext(1);
    auto ground_task = p::Task<p::LiftedTag>(fp::Parser(param.domain_file).parse_task(param.task_file)).instantiate_ground_task(execution_context).task;

    auto shared_execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::GroundTag>().create(ground_task, shared_execution_context);
    auto state_repository = p::StateRepositoryFactory<p::GroundTag>().create(ground_task, axiom_evaluator);

    const auto initial_state = state_repository->get_initial_state();

    auto fluent_facts = std::vector<ygg::Data<fp::FDRFact<f::FluentTag>>> {};
    for (const auto fact : initial_state.get_fluent_facts())
        fluent_facts.push_back(fact);

    auto fluent_fact_views = std::vector<fp::FDRFactView<f::FluentTag>> {};
    for (const auto fact : initial_state.get_fluent_facts_view())
        fluent_fact_views.push_back(fact);

    auto fterm_values = std::vector<std::pair<ygg::Index<fp::GroundFunctionTerm<f::FluentTag>>, ygg::float_t>> {};
    for (const auto value : initial_state.get_fluent_fterm_values())
        fterm_values.push_back(value);

    auto fterm_value_views = std::vector<fp::GroundFunctionTermViewValuePair<f::FluentTag>> {};
    for (const auto value : initial_state.get_fluent_fterm_values_view())
        fterm_value_views.push_back(value);

    const auto data_state = state_repository->create_state(fluent_facts, fterm_values);
    const auto view_state = state_repository->create_state(fluent_fact_views, fterm_value_views);

    EXPECT_EQ(data_state.get_index(), initial_state.get_index());
    EXPECT_EQ(view_state.get_index(), initial_state.get_index());
    EXPECT_TRUE(ygg::EqualTo<p::StateView<p::GroundTag>> {}(data_state, initial_state));
    EXPECT_TRUE(ygg::EqualTo<p::StateView<p::GroundTag>> {}(view_state, initial_state));
    EXPECT_EQ(state_repository->num_states(), 1);
}

TEST_P(GroundTaskTest, LabeledSuccessorOutputBufferIsReplaced)
{
    const auto& param = GetParam();
    auto execution_context = ygg::ExecutionContext(1);
    auto ground_task = p::Task<p::LiftedTag>(fp::Parser(param.domain_file).parse_task(param.task_file)).instantiate_ground_task(execution_context).task;

    auto successor_execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::GroundTag>().create(ground_task, successor_execution_context);
    auto state_repository = p::StateRepositoryFactory<p::GroundTag>().create(ground_task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::GroundTag>().create(ground_task, successor_execution_context, state_repository);
    const auto initial_node = successor_generator->get_initial_node();

    auto out_nodes = p::LabeledNodeList<p::GroundTag> {};
    successor_generator->get_labeled_successor_nodes(initial_node, out_nodes);
    ASSERT_EQ(out_nodes.size(), param.expected_successors);
    ASSERT_FALSE(out_nodes.empty());

    out_nodes.push_back(out_nodes.front());
    ASSERT_EQ(out_nodes.size(), param.expected_successors + 1);

    successor_generator->get_labeled_successor_nodes(initial_node, out_nodes);
    EXPECT_EQ(out_nodes.size(), param.expected_successors);
}

TEST_P(GroundTaskTest, HasExpectedGroundTaskAndSuccessorCounts)
{
    const auto& param = GetParam();
    auto execution_context = ygg::ExecutionContext(1);
    auto ground_task = p::Task<p::LiftedTag>(fp::Parser(param.domain_file).parse_task(param.task_file)).instantiate_ground_task(execution_context).task;

    EXPECT_EQ(ground_task->get_num_atoms<f::FluentTag>(), param.expected_fluent_atoms);
    EXPECT_EQ(ground_task->get_num_atoms<f::DerivedTag>(), param.expected_derived_atoms);
    EXPECT_EQ(ground_task->get_num_actions(), param.expected_actions);
    EXPECT_EQ(ground_task->get_num_axioms(), param.expected_axioms);

    auto successor_execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::GroundTag>().create(ground_task, successor_execution_context);
    auto state_repository = p::StateRepositoryFactory<p::GroundTag>().create(ground_task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::GroundTag>().create(ground_task, successor_execution_context, state_repository);

    EXPECT_EQ(successor_generator->get_labeled_successor_nodes(successor_generator->get_initial_node()).size(), param.expected_successors);
}

INSTANTIATE_TEST_SUITE_P(TyrPlanningGroundTask,
                         GroundTaskTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<GroundTaskCase>& info) { return info.param.name; });
}
