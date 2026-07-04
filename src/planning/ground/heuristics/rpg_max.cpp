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

#include "tyr/planning/ground/heuristics/rpg_max.hpp"

namespace tyr::planning
{

MaxRPGHeuristic<GroundTag>::MaxRPGHeuristic(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode) :
    RPGBase<GroundTag,
            MaxRPGHeuristic<GroundTag>,
            datalog::OrAnnotationPolicy<GroundTag>,
            datalog::AndAnnotationPolicy<GroundTag, datalog::MaxAggregation>,
            datalog::TerminationPolicy<GroundTag, datalog::MaxAggregation>,
            datalog::RuleCostOverridePolicy<GroundTag>>(std::move(task),
                                                        std::move(execution_context),
                                                        datalog::OrAnnotationPolicy<GroundTag>(),
                                                        datalog::AndAnnotationPolicy<GroundTag, datalog::MaxAggregation>(),
                                                        cost_mode)
{
}

MaxRPGHeuristicPtr<GroundTag> MaxRPGHeuristic<GroundTag>::create(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode)
{
    return std::make_shared<MaxRPGHeuristic<GroundTag>>(std::move(task), std::move(execution_context), cost_mode);
}

ygg::float_t MaxRPGHeuristic<GroundTag>::extract_cost_and_set_preferred_actions_impl(const StateView<GroundTag>&) { return get_goal_cost(); }

}
