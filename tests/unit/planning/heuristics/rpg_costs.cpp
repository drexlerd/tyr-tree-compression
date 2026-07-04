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

#include "tyr/datalog/lifted/policies/annotation.hpp"
#include "tyr/datalog/lifted/policies/cost.hpp"
#include "tyr/datalog/lifted/policies/termination.hpp"
#include "tyr/formalism/datalog/expression_properties.hpp"
#include "tyr/formalism/formalism.hpp"
#include "tyr/planning/ground/programs/rpg.hpp"
#include "tyr/planning/lifted/heuristics/rpg.hpp"
#include "tyr/planning/lifted/programs/rpg.hpp"
#include "tyr/planning/planning.hpp"

#include <algorithm>
#include <filesystem>
#include <gtest/gtest.h>
#include <optional>
#include <vector>
#include <yggdrasil/containers/unordered_set.hpp>

namespace d = tyr::datalog;
namespace f = tyr::formalism;
namespace fd = tyr::formalism::datalog;
namespace fp = tyr::formalism::planning;
namespace p = tyr::planning;

namespace tyr::tests
{
namespace
{
class TestCostAdaptedMaxRPG :
    public p::RPGBase<p::LiftedTag,
                      TestCostAdaptedMaxRPG,
                      d::OrAnnotationPolicy<LiftedTag>,
                      d::AndAnnotationPolicy<LiftedTag, d::MaxAggregation>,
                      d::TerminationPolicy<LiftedTag, d::MaxAggregation>,
                      d::RuleCostOverridePolicy<LiftedTag>>
{
public:
    using Base = p::RPGBase<p::LiftedTag,
                            TestCostAdaptedMaxRPG,
                            d::OrAnnotationPolicy<LiftedTag>,
                            d::AndAnnotationPolicy<LiftedTag, d::MaxAggregation>,
                            d::TerminationPolicy<LiftedTag, d::MaxAggregation>,
                            d::RuleCostOverridePolicy<LiftedTag>>;

    TestCostAdaptedMaxRPG(p::TaskPtr<p::LiftedTag> task, ygg::ExecutionContextPtr execution_context) :
        Base(task, std::move(execution_context), d::OrAnnotationPolicy<LiftedTag>(), d::AndAnnotationPolicy<LiftedTag, d::MaxAggregation>())
    {
    }

    using Base::get_workspace;
    using Base::set_action_binding_cost;

    ygg::float_t extract_cost_and_set_preferred_actions_impl(const p::StateView<p::LiftedTag>&) { return 0; }
};

size_t count_rules_with_metric_effects(const p::RPGProgram<p::LiftedTag>& program)
{
    size_t result = 0;
    for (const auto rule : program.get_datalog_program().get_program().get_rules())
        if (!rule.get_metric_effects().empty())
            ++result;
    return result;
}

size_t count_rules_with_metric_effects(const p::RPGProgram<p::GroundTag>& program)
{
    size_t result = 0;
    for (const auto rule : program.get_datalog_program().get_program().get_ground_rules())
        if (!rule.get_metric_effects().empty())
            ++result;
    return result;
}

bool targets_function(fd::NumericEffectOperatorView<f::FluentTag> effect, const ygg::UnorderedSet<fd::FunctionView<f::FluentTag>>& functions)
{
    return ygg::visit([&](auto&& arg) { return functions.find(arg.get_fterm().get_function()) != functions.end(); }, effect.get_variant());
}

bool targets_fterm(fd::GroundNumericEffectOperatorView<f::FluentTag> effect, const ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& fterms)
{
    return ygg::visit([&](auto&& arg) { return fterms.find(arg.get_fterm()) != fterms.end(); }, effect.get_variant());
}

size_t count_rules_for_action(const TestCostAdaptedMaxRPG& heuristic, fp::ActionView action)
{
    return std::ranges::count_if(heuristic.get_rpg_program().get_rule_to_action_mapping(),
                                 [&](const auto& entry) { return entry.second.get_index() == action.get_index(); });
}

TEST(TyrPlanningRPGCostsTest, ActionBindingCostOverridesAllRPGRuleBindingsForAction)
{
    const auto root = std::filesystem::path(ROOT_DIR);
    auto task = p::Task<p::LiftedTag>::create(fp::Parser(root / "data/planning-benchmarks/tests/classical/blocks_3/domain.pddl")
                                                  .parse_task(root / "data/planning-benchmarks/tests/classical/blocks_3/test-1.pddl"));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::LiftedTag>().create(task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::LiftedTag>().create(task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::LiftedTag>().create(task, execution_context, state_repository);
    const auto initial_node = successor_generator->get_initial_node();
    const auto action_bindings = successor_generator->get_applicable_action_bindings(initial_node);

    auto heuristic = TestCostAdaptedMaxRPG(task, execution_context);

    auto selected_binding = std::optional<fp::ActionBindingView>();
    for (const auto binding : action_bindings)
    {
        if (count_rules_for_action(heuristic, binding.get_relation()) > 1)
        {
            selected_binding = binding;
            break;
        }
    }
    ASSERT_TRUE(selected_binding.has_value());

    heuristic.set_action_binding_cost(*selected_binding, 5);

    const auto expected_num_rules = count_rules_for_action(heuristic, selected_binding->get_relation());
    EXPECT_TRUE(heuristic.get_workspace().cost_policy.get_costs().empty());
    EXPECT_EQ(heuristic.get_workspace().cost_policy.get_num_prefix_costs(), expected_num_rules);
}

TEST(TyrPlanningRPGCostsTest, LiftedRPGMetricEffectsAreMetricFiltered)
{
    const auto root = std::filesystem::path(ROOT_DIR);
    const auto task = fp::Parser(root / "data/planning-benchmarks/tests/numeric/delivery/domain.pddl")
                          .parse_task(root / "data/planning-benchmarks/tests/numeric/delivery/test-1.pddl");
    auto program = p::RPGProgram<p::LiftedTag>(task.get_task());

    auto metric_fterms = ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>> {};
    fd::collect_fterms(program.get_datalog_program().get_program().get_metric().value().get_fexpr(), metric_fterms);
    auto metric_functions = ygg::UnorderedSet<fd::FunctionView<f::FluentTag>> {};
    for (const auto fterm : metric_fterms)
        metric_functions.insert(fterm.get_function());

    ASSERT_GT(count_rules_with_metric_effects(program), 0);
    for (const auto rule : program.get_datalog_program().get_program().get_rules())
    {
        for (const auto metric_effect : rule.get_metric_effects())
            EXPECT_TRUE(targets_function(metric_effect, metric_functions));
    }
}

TEST(TyrPlanningRPGCostsTest, GroundRPGMetricEffectsAreMetricFiltered)
{
    const auto root = std::filesystem::path(ROOT_DIR);
    auto lifted_task = p::Task<p::LiftedTag>(fp::Parser(root / "data/planning-benchmarks/tests/numeric/delivery/domain.pddl")
                                                 .parse_task(root / "data/planning-benchmarks/tests/numeric/delivery/test-1.pddl"));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto ground_task = lifted_task.instantiate_ground_task(*execution_context).task;
    ASSERT_NE(ground_task, nullptr);

    auto program = p::RPGProgram<p::GroundTag>(ground_task->get_task());
    auto metric_fterms = ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>> {};
    fd::collect_fterms(program.get_datalog_program().get_program().get_metric().value().get_fexpr(), metric_fterms);

    ASSERT_GT(count_rules_with_metric_effects(program), 0);
    for (const auto rule : program.get_datalog_program().get_program().get_ground_rules())
    {
        for (const auto metric_effect : rule.get_metric_effects())
            EXPECT_TRUE(targets_fterm(metric_effect, metric_fterms));
    }
}

}

}
