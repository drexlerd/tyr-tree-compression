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

#ifndef TYR_PLANNING_GROUND_HEURISTICS_MAX_HPP_
#define TYR_PLANNING_GROUND_HEURISTICS_MAX_HPP_

#include "tyr/datalog/ground/policies/annotation.hpp"
#include "tyr/datalog/ground/policies/termination.hpp"
#include "tyr/planning/ground/heuristics/rpg.hpp"
#include "tyr/planning/heuristics/rpg_max.hpp"

namespace tyr::planning
{

template<>
class MaxRPGHeuristic<GroundTag> :
    public RPGBase<GroundTag,
                   MaxRPGHeuristic<GroundTag>,
                   datalog::OrAnnotationPolicy<GroundTag>,
                   datalog::AndAnnotationPolicy<GroundTag, datalog::MaxAggregation>,
                   datalog::TerminationPolicy<GroundTag, datalog::MaxAggregation>>
{
public:
    MaxRPGHeuristic(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context);

    static MaxRPGHeuristicPtr<GroundTag> create(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context);

    ygg::float_t extract_cost_and_set_preferred_actions_impl(const StateView<GroundTag>& state);
};

}

#endif
