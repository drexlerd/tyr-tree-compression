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

#include "planning/parser.hpp"

#include <algorithm>
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
struct LiftedSuccessorCountCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
    size_t expected_successors;
};

LiftedSuccessorCountCase parse_case(const boost::json::object& suite, const boost::json::object& object)
{
    return LiftedSuccessorCountCase { ygg::common::as_string(object, "name", "case"),
                                      ygg::common::suite_path(suite, ygg::common::as_string(object, "domain_file", "case")),
                                      ygg::common::suite_path(suite, ygg::common::as_string(object, "task_file", "case")),
                                      ygg::common::as_size(object, "expected_successors", "case") };
}

std::vector<LiftedSuccessorCountCase> load_cases()
{
    const auto suite = ygg::common::load_json_file(ygg::common::root_path() / "tests/unit/planning/lifted_task.json");
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    auto result = std::vector<LiftedSuccessorCountCase> {};
    for (const auto& case_value : ygg::common::as_array(suite_object, "cases", "suite"))
        result.push_back(parse_case(suite_object, ygg::common::as_object(case_value, "case")));
    return result;
}

void expect_same_node(const p::Node<p::LiftedTag>& expected, const p::Node<p::LiftedTag>& actual)
{
    EXPECT_EQ(ygg::uint_t(expected.get_state().get_index()), ygg::uint_t(actual.get_state().get_index()));
    EXPECT_TRUE(f::apply(f::Eq {}, expected.get_metric(), actual.get_metric()))
        << "expected metric " << expected.get_metric() << ", actual metric " << actual.get_metric();
}

void expect_same_binding(fp::ActionBindingView expected, fp::ActionBindingView actual)
{
    EXPECT_EQ(ygg::uint_t(expected.get_relation().get_index()), ygg::uint_t(actual.get_relation().get_index()));

    const auto expected_objects = expected.get_data();
    const auto actual_objects = actual.get_data();
    ASSERT_EQ(expected_objects.size(), actual_objects.size());
    for (size_t i = 0; i < expected_objects.size(); ++i)
        EXPECT_EQ(ygg::uint_t(expected_objects[i]), ygg::uint_t(actual_objects[i]));
}

void expect_same_binding(fp::ActionBindingView expected, const ygg::Data<::tyr::formalism::RelationBinding<::tyr::formalism::planning::Action>>& actual)
{
    EXPECT_EQ(ygg::uint_t(expected.get_relation().get_index()), ygg::uint_t(actual.relation));

    const auto expected_objects = expected.get_data();
    ASSERT_EQ(expected_objects.size(), actual.objects.size());
    for (size_t i = 0; i < expected_objects.size(); ++i)
        EXPECT_EQ(ygg::uint_t(expected_objects[i]), ygg::uint_t(actual.objects[i]));
}

void expect_ground_action_objects_match_binding(fp::GroundActionView ground_action, fp::ActionBindingView binding)
{
    EXPECT_EQ(ygg::uint_t(ground_action.get_action().get_index()), ygg::uint_t(binding.get_relation().get_index()));

    const auto objects = ground_action.get_objects();
    const auto binding_objects = binding.get_data();
    ASSERT_EQ(objects.size(), binding_objects.size());
    for (size_t i = 0; i < objects.size(); ++i)
        EXPECT_EQ(ygg::uint_t(objects[i].get_index()), ygg::uint_t(binding_objects[i]));
}

void expect_ground_action_objects_match_binding(fp::GroundActionView ground_action,
                                                const ygg::Data<::tyr::formalism::RelationBinding<::tyr::formalism::planning::Action>>& binding)
{
    EXPECT_EQ(ygg::uint_t(ground_action.get_action().get_index()), ygg::uint_t(binding.relation));

    const auto objects = ground_action.get_objects();
    ASSERT_EQ(objects.size(), binding.objects.size());
    for (size_t i = 0; i < objects.size(); ++i)
        EXPECT_EQ(ygg::uint_t(objects[i].get_index()), ygg::uint_t(binding.objects[i]));
}

bool are_same_binding(fp::ActionBindingView lhs, fp::ActionBindingView rhs)
{
    if (lhs.get_relation().get_index() != rhs.get_relation().get_index())
        return false;

    const auto lhs_objects = lhs.get_data();
    const auto rhs_objects = rhs.get_data();
    return std::ranges::equal(lhs_objects, rhs_objects);
}

bool are_same_binding(fp::ActionBindingView lhs, const ygg::Data<::tyr::formalism::RelationBinding<::tyr::formalism::planning::Action>>& rhs)
{
    if (lhs.get_relation().get_index() != rhs.relation)
        return false;

    return std::ranges::equal(lhs.get_data(), rhs.objects);
}

void expect_action_binding_apis_match_ground_actions(const LiftedSuccessorCountCase& test_case)
{
    auto lifted_task = p::Task<p::LiftedTag>::create(make_test_parser(test_case.domain_file).parse_task(test_case.task_file));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::LiftedTag>().create(lifted_task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::LiftedTag>().create(lifted_task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::LiftedTag>().create(lifted_task, execution_context, state_repository);
    const auto initial_node = successor_generator->get_initial_node();

    const auto ground_successors = successor_generator->get_labeled_successor_nodes(initial_node);
    const auto interned_bindings = successor_generator->get_applicable_action_bindings(initial_node);

    ASSERT_EQ(ground_successors.size(), interned_bindings.size());
    for (const auto binding : interned_bindings)
    {
        const auto expected =
            std::ranges::find_if(ground_successors, [&](const auto& successor) { return are_same_binding(successor.label.get_row(), binding); });
        ASSERT_NE(expected, ground_successors.end());

        expect_same_binding(expected->label.get_row(), binding);
        expect_ground_action_objects_match_binding(expected->label, binding);
        expect_same_node(expected->node, successor_generator->get_successor_node(initial_node, binding));
    }

    size_t no_interning_pos = 0;
    successor_generator->for_each_applicable_action_binding(
        initial_node,
        [&](const auto& binding)
        {
            const auto expected =
                std::ranges::find_if(ground_successors, [&](const auto& successor) { return are_same_binding(successor.label.get_row(), binding); });
            ASSERT_NE(expected, ground_successors.end());

            expect_same_binding(expected->label.get_row(), binding);
            expect_ground_action_objects_match_binding(expected->label, binding);
            expect_same_node(expected->node, successor_generator->get_successor_node(initial_node, binding));
            ++no_interning_pos;
        });

    EXPECT_EQ(no_interning_pos, ground_successors.size());
}
}

class LiftedTaskSuccessorCountTest : public ::testing::TestWithParam<LiftedSuccessorCountCase>
{
};

TEST(LiftedTaskGrounderResultTest, DefaultConstructionRepresentsExplicitFailure)
{
    const auto result = p::GroundTaskInstantiationResult {};

    EXPECT_EQ(result.task, nullptr);
    EXPECT_EQ(result.status, p::GroundTaskInstantiationStatus::PROVEN_UNSOLVABLE);
}

TEST_P(LiftedTaskSuccessorCountTest, InitialNodeHasExpectedSuccessorCount)
{
    const auto& param = GetParam();
    auto lifted_task = p::Task<p::LiftedTag>::create(make_test_parser(param.domain_file).parse_task(param.task_file));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::LiftedTag>().create(lifted_task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::LiftedTag>().create(lifted_task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::LiftedTag>().create(lifted_task, execution_context, state_repository);

    EXPECT_EQ(successor_generator->get_labeled_successor_nodes(successor_generator->get_initial_node()).size(), param.expected_successors);
}

TEST_P(LiftedTaskSuccessorCountTest, ActionBindingApisMatchGroundActions) { expect_action_binding_apis_match_ground_actions(GetParam()); }

TEST_P(LiftedTaskSuccessorCountTest, CreateStateOverloadsCanonicalizeToSameRegisteredState)
{
    const auto& param = GetParam();
    auto lifted_task = p::Task<p::LiftedTag>::create(make_test_parser(param.domain_file).parse_task(param.task_file));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::LiftedTag>().create(lifted_task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::LiftedTag>().create(lifted_task, axiom_evaluator);

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
    EXPECT_TRUE(ygg::EqualTo<p::StateView<p::LiftedTag>> {}(data_state, initial_state));
    EXPECT_TRUE(ygg::EqualTo<p::StateView<p::LiftedTag>> {}(view_state, initial_state));
    EXPECT_EQ(state_repository->num_states(), 1);
}

TEST_P(LiftedTaskSuccessorCountTest, LabeledSuccessorOutputBufferIsReplaced)
{
    const auto& param = GetParam();
    auto lifted_task = p::Task<p::LiftedTag>::create(make_test_parser(param.domain_file).parse_task(param.task_file));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::LiftedTag>().create(lifted_task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::LiftedTag>().create(lifted_task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::LiftedTag>().create(lifted_task, execution_context, state_repository);
    const auto initial_node = successor_generator->get_initial_node();

    auto out_nodes = p::LabeledNodeList<p::LiftedTag> {};
    successor_generator->get_labeled_successor_nodes(initial_node, out_nodes);
    ASSERT_EQ(out_nodes.size(), param.expected_successors);
    ASSERT_FALSE(out_nodes.empty());

    out_nodes.push_back(out_nodes.front());
    ASSERT_EQ(out_nodes.size(), param.expected_successors + 1);

    successor_generator->get_labeled_successor_nodes(initial_node, out_nodes);
    EXPECT_EQ(out_nodes.size(), param.expected_successors);
}

TEST_P(LiftedTaskSuccessorCountTest, ApplicableActionBindingOutputBufferIsReplaced)
{
    const auto& param = GetParam();
    auto lifted_task = p::Task<p::LiftedTag>::create(make_test_parser(param.domain_file).parse_task(param.task_file));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::LiftedTag>().create(lifted_task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::LiftedTag>().create(lifted_task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::LiftedTag>().create(lifted_task, execution_context, state_repository);
    const auto initial_node = successor_generator->get_initial_node();

    auto out_bindings = std::vector<fp::ActionBindingView> {};
    successor_generator->get_applicable_action_bindings(initial_node, out_bindings);
    ASSERT_EQ(out_bindings.size(), param.expected_successors);
    ASSERT_FALSE(out_bindings.empty());

    out_bindings.push_back(out_bindings.front());
    ASSERT_EQ(out_bindings.size(), param.expected_successors + 1);

    successor_generator->get_applicable_action_bindings(initial_node, out_bindings);
    EXPECT_EQ(out_bindings.size(), param.expected_successors);
}

TEST_P(LiftedTaskSuccessorCountTest, StateViewsUseRepositoryContextForIdentity)
{
    const auto& param = GetParam();
    auto lifted_task = p::Task<p::LiftedTag>::create(make_test_parser(param.domain_file).parse_task(param.task_file));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator_factory = p::AxiomEvaluatorFactory<p::LiftedTag>();
    auto state_repository_factory = p::StateRepositoryFactory<p::LiftedTag>();
    auto successor_generator_factory = p::SuccessorGeneratorFactory<p::LiftedTag>();

    auto first_axiom_evaluator = axiom_evaluator_factory.create(lifted_task, execution_context);
    auto second_axiom_evaluator = axiom_evaluator_factory.create(lifted_task, execution_context);

    auto first_repository = state_repository_factory.create(lifted_task, first_axiom_evaluator);
    auto second_repository = state_repository_factory.create(lifted_task, second_axiom_evaluator);

    auto first_successor_generator = successor_generator_factory.create(lifted_task, execution_context, first_repository);
    auto second_successor_generator = successor_generator_factory.create(lifted_task, execution_context, second_repository);

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
    EXPECT_FALSE(ygg::EqualTo<p::StateView<p::LiftedTag>> {}(first_state, second_state));
    EXPECT_NE(ygg::Hash<p::StateView<p::LiftedTag>> {}(first_state), ygg::Hash<p::StateView<p::LiftedTag>> {}(second_state));
}

TEST_P(LiftedTaskSuccessorCountTest, StateViewsFromIndependentFactoriesUseDeterministicFactoryLocalIdentity)
{
    const auto& param = GetParam();
    auto lifted_task = p::Task<p::LiftedTag>::create(make_test_parser(param.domain_file).parse_task(param.task_file));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator_factory = p::AxiomEvaluatorFactory<p::LiftedTag>();
    auto first_state_repository_factory = p::StateRepositoryFactory<p::LiftedTag>();
    auto second_state_repository_factory = p::StateRepositoryFactory<p::LiftedTag>();

    auto first_axiom_evaluator = axiom_evaluator_factory.create(lifted_task, execution_context);
    auto second_axiom_evaluator = axiom_evaluator_factory.create(lifted_task, execution_context);

    auto first_repository = first_state_repository_factory.create(lifted_task, first_axiom_evaluator);
    auto second_repository = second_state_repository_factory.create(lifted_task, second_axiom_evaluator);

    const auto first_state = first_repository->get_initial_state();
    const auto second_state = second_repository->get_initial_state();

    EXPECT_EQ(first_repository->get_index(), 0);
    EXPECT_EQ(second_repository->get_index(), 0);
    EXPECT_EQ(first_state.get_index(), second_state.get_index());
    EXPECT_TRUE(ygg::EqualTo<p::StateView<p::LiftedTag>> {}(first_state, second_state));
    EXPECT_EQ(ygg::Hash<p::StateView<p::LiftedTag>> {}(first_state), ygg::Hash<p::StateView<p::LiftedTag>> {}(second_state));
}

INSTANTIATE_TEST_SUITE_P(TyrPlanningLiftedTask,
                         LiftedTaskSuccessorCountTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<LiftedSuccessorCountCase>& info) { return info.param.name; });
}
