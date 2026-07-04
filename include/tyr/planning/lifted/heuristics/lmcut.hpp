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

#ifndef TYR_PLANNING_LIFTED_HEURISTICS_LMCUT_HPP_
#define TYR_PLANNING_LIFTED_HEURISTICS_LMCUT_HPP_

#include "tyr/datalog/lifted/policies/annotation.hpp"
#include "tyr/datalog/lifted/policies/cost.hpp"
#include "tyr/datalog/lifted/policies/termination.hpp"
#include "tyr/planning/heuristics/lmcut.hpp"
#include "tyr/planning/lifted/heuristics/rpg.hpp"

#include <deque>
#include <tuple>
#include <variant>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace tyr::planning
{

struct LiftedLMCutNumericNode
{
    ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> binding;
    ygg::ClosedInterval<ygg::float_t> interval;

    auto identifying_members() const noexcept { return std::make_tuple(binding, lower(interval), upper(interval)); }
};

struct LiftedLMCutRuleEdge
{
    ygg::Index<::tyr::formalism::datalog::Rule> rule;
    ygg::IndexList<::tyr::formalism::Object> objects;

    auto identifying_members() const noexcept { return std::tie(rule, objects); }
};

struct LiftedLMCutNumericEdge
{
    ygg::Index<::tyr::formalism::datalog::Rule> rule;
    ygg::IndexList<::tyr::formalism::Object> rule_objects;
    ygg::Index<::tyr::formalism::Function<::tyr::formalism::FluentTag>> function;
    ygg::IndexList<::tyr::formalism::Object> function_objects;
    ygg::ClosedInterval<ygg::float_t> interval;

    auto identifying_members() const noexcept { return std::make_tuple(rule, rule_objects, function, function_objects, lower(interval), upper(interval)); }
};

template<>
class LMCutHeuristic<LiftedTag> :
    public RPGBase<LiftedTag,
                   LMCutHeuristic<LiftedTag>,
                   datalog::OrAnnotationPolicy<LiftedTag>,
                   datalog::AchieverAndAnnotationPolicy<LiftedTag, datalog::MaxAggregation>,
                   datalog::TerminationPolicy<LiftedTag, datalog::MaxAggregation>,
                   datalog::RuleCostOverridePolicy<LiftedTag>>
{
public:
    using Base = RPGBase<LiftedTag,
                         LMCutHeuristic<LiftedTag>,
                         datalog::OrAnnotationPolicy<LiftedTag>,
                         datalog::AchieverAndAnnotationPolicy<LiftedTag, datalog::MaxAggregation>,
                         datalog::TerminationPolicy<LiftedTag, datalog::MaxAggregation>,
                         datalog::RuleCostOverridePolicy<LiftedTag>>;

    LMCutHeuristic(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode = CostMode::GENERAL);

    static LMCutHeuristicPtr<LiftedTag> create(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode = CostMode::GENERAL);

    ygg::float_t evaluate(const StateView<LiftedTag>& state) override;

    ygg::float_t extract_cost_and_set_preferred_actions_impl(const StateView<LiftedTag>& state);

private:
    using ActionBinding = ::tyr::formalism::planning::ActionBindingView;
    using PredicateBinding = ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag>;

    using NumericNode = LiftedLMCutNumericNode;
    using RuleEdge = LiftedLMCutRuleEdge;
    using NumericEdge = LiftedLMCutNumericEdge;
    using Precondition = std::variant<PredicateBinding, NumericNode>;

    RuleEdge make_rule_edge(const datalog::WitnessAnnotation<LiftedTag>& witness) const;
    NumericEdge make_numeric_edge(const datalog::WitnessAnnotation<LiftedTag>& witness, NumericNode node) const;
    datalog::Cost get_residual_cost(ActionBinding action_binding) const;
    datalog::Cost get_witness_body_cost(const datalog::WitnessAnnotation<LiftedTag>& witness);
    datalog::Cost get_witness_edge_residual_cost(const datalog::WitnessAnnotation<LiftedTag>& witness);
    void set_residual_cost(ActionBinding action_binding, datalog::Cost cost);
    void use_rule_edge_cost(RuleEdge edge, datalog::Cost cost);
    void use_numeric_edge_cost(NumericEdge edge, datalog::Cost cost);
    void apply_residual_costs();
    datalog::Cost get_numeric_cost(NumericNode node) const noexcept;
    const datalog::WitnessAnnotation<LiftedTag>* get_numeric_witness(NumericNode node) const noexcept;
    const std::vector<Precondition>& get_witness_max_preconditions(const datalog::WitnessAnnotation<LiftedTag>& witness, datalog::Cost edge_cost);
    void release_witness_max_preconditions();
    void mark_goal_zone(PredicateBinding binding);
    void mark_goal_zone(NumericNode node);
    void mark_goal_zone(Precondition precondition);
    bool is_before_goal_zone(PredicateBinding binding);
    bool is_before_goal_zone(NumericNode node);
    bool is_before_goal_zone(Precondition precondition);
    void extract_cut();
    void extract_expanded_cut();

    ygg::UnorderedMap<ActionBinding, datalog::Cost> m_residual_costs;
    ygg::UnorderedMap<RuleEdge, datalog::Cost> m_rule_edge_used_costs;
    ygg::UnorderedMap<NumericEdge, datalog::Cost> m_numeric_edge_used_costs;
    ygg::UnorderedSet<PredicateBinding> m_goal_zone;
    ygg::UnorderedSet<NumericNode> m_numeric_goal_zone;
    ygg::UnorderedSet<PredicateBinding> m_before_goal_zone;
    ygg::UnorderedSet<NumericNode> m_numeric_before_goal_zone;
    ygg::UnorderedSet<PredicateBinding> m_not_before_goal_zone;
    ygg::UnorderedSet<NumericNode> m_numeric_not_before_goal_zone;
    ygg::UnorderedSet<ActionBinding> m_cut;
    ygg::UnorderedMap<RuleEdge, datalog::Cost> m_rule_cut;
    ygg::UnorderedMap<NumericEdge, datalog::Cost> m_numeric_cut;
    std::deque<std::vector<Precondition>> m_max_precondition_buffers;
    size_t m_max_precondition_depth;
    bool m_use_expanded_edges;
};

}

#endif
