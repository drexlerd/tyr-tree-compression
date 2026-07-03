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
#include <vector>

namespace tyr::planning
{

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

    LMCutHeuristic(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context);

    static LMCutHeuristicPtr<LiftedTag> create(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context);

    ygg::float_t evaluate(const StateView<LiftedTag>& state) override;

    ygg::float_t extract_cost_and_set_preferred_actions_impl(const StateView<LiftedTag>& state);

private:
    using ActionBinding = ::tyr::formalism::planning::ActionBindingView;
    using PredicateBinding = ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag>;

    datalog::Cost get_residual_cost(ActionBinding action_binding) const;
    void set_residual_cost(ActionBinding action_binding, datalog::Cost cost);
    void apply_residual_costs();
    const std::vector<PredicateBinding>& get_witness_max_preconditions(const datalog::WitnessAnnotation& witness);
    void release_witness_max_preconditions();
    void mark_goal_zone(PredicateBinding binding);
    bool is_before_goal_zone(PredicateBinding binding);
    void extract_cut();

    ygg::UnorderedMap<ActionBinding, datalog::Cost> m_residual_costs;
    ygg::UnorderedSet<PredicateBinding> m_goal_zone;
    ygg::UnorderedSet<PredicateBinding> m_before_goal_zone;
    ygg::UnorderedSet<PredicateBinding> m_not_before_goal_zone;
    ygg::UnorderedSet<ActionBinding> m_cut;
    std::deque<std::vector<PredicateBinding>> m_max_precondition_buffers;
    size_t m_max_precondition_depth;
};

}

#endif
