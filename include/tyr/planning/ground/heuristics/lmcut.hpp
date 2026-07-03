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

#ifndef TYR_PLANNING_GROUND_HEURISTICS_LMCUT_HPP_
#define TYR_PLANNING_GROUND_HEURISTICS_LMCUT_HPP_

#include "tyr/datalog/ground/policies/annotation.hpp"
#include "tyr/datalog/ground/policies/cost.hpp"
#include "tyr/datalog/ground/policies/termination.hpp"
#include "tyr/planning/ground/heuristics/rpg.hpp"
#include "tyr/planning/heuristics/lmcut.hpp"

#include <deque>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>

namespace tyr::planning
{

template<>
class LMCutHeuristic<GroundTag> :
    public RPGBase<GroundTag,
                   LMCutHeuristic<GroundTag>,
                   datalog::OrAnnotationPolicy<GroundTag>,
                   datalog::AchieverAndAnnotationPolicy<GroundTag, datalog::MaxAggregation>,
                   datalog::TerminationPolicy<GroundTag, datalog::MaxAggregation>,
                   datalog::RuleCostOverridePolicy<GroundTag>>
{
public:
    using Base = RPGBase<GroundTag,
                         LMCutHeuristic<GroundTag>,
                         datalog::OrAnnotationPolicy<GroundTag>,
                         datalog::AchieverAndAnnotationPolicy<GroundTag, datalog::MaxAggregation>,
                         datalog::TerminationPolicy<GroundTag, datalog::MaxAggregation>,
                         datalog::RuleCostOverridePolicy<GroundTag>>;

    LMCutHeuristic(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context);

    static LMCutHeuristicPtr<GroundTag> create(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context);

    ygg::float_t evaluate(const StateView<GroundTag>& state) override;

    ygg::float_t extract_cost_and_set_preferred_actions_impl(const StateView<GroundTag>& state);

private:
    using Action = ::tyr::formalism::planning::GroundActionView;
    using Atom = ::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>;

    datalog::Cost get_residual_cost(Action action) const;
    void set_residual_cost(Action action, datalog::Cost cost);
    void apply_residual_costs();
    const std::vector<Atom>& get_witness_max_preconditions(const datalog::GroundWitnessAnnotation& witness);
    void release_witness_max_preconditions();
    void mark_goal_zone(Atom atom);
    bool is_before_goal_zone(Atom atom);
    void extract_cut();

    ygg::UnorderedMap<Action, datalog::Cost> m_residual_costs;
    ygg::UnorderedSet<Atom> m_goal_zone;
    ygg::UnorderedSet<Atom> m_before_goal_zone;
    ygg::UnorderedSet<Atom> m_not_before_goal_zone;
    ygg::UnorderedSet<Action> m_cut;
    std::deque<std::vector<Atom>> m_max_precondition_buffers;
    size_t m_max_precondition_depth;
};

}

#endif
