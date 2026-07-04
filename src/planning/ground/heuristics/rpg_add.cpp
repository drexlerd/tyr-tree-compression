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

#include "tyr/planning/ground/heuristics/rpg_add.hpp"

namespace tyr::planning
{

AddRPGHeuristic<GroundTag>::AddRPGHeuristic(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context) :
    RPGBase<GroundTag,
            AddRPGHeuristic<GroundTag>,
            datalog::OrAnnotationPolicy<GroundTag>,
            datalog::AndAnnotationPolicy<GroundTag, datalog::SumAggregation>,
            datalog::TerminationPolicy<GroundTag, datalog::SumAggregation>,
            datalog::RuleCostOverridePolicy<GroundTag>>(std::move(task),
                                                        std::move(execution_context),
                                                        datalog::OrAnnotationPolicy<GroundTag>(),
                                                        datalog::AndAnnotationPolicy<GroundTag, datalog::SumAggregation>())
{
}

AddRPGHeuristicPtr<GroundTag> AddRPGHeuristic<GroundTag>::create(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context)
{
    return std::make_shared<AddRPGHeuristic<GroundTag>>(std::move(task), std::move(execution_context));
}

ygg::float_t AddRPGHeuristic<GroundTag>::extract_cost_and_set_preferred_actions_impl(const StateView<GroundTag>&) { return get_goal_cost(); }

}
