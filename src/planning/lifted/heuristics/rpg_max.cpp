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

#include "tyr/planning/lifted/heuristics/rpg_max.hpp"

#include "tyr/datalog/lifted/policies/annotation.hpp"
#include "tyr/datalog/policies/termination.hpp"
#include "tyr/formalism/planning/merge_datalog.hpp"
#include "tyr/planning/lifted/heuristics/rpg.hpp"

namespace tyr::planning
{

MaxRPGHeuristic<LiftedTag>::MaxRPGHeuristic(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode) :
    RPGBase<LiftedTag,
            MaxRPGHeuristic<LiftedTag>,
            datalog::OrAnnotationPolicy<LiftedTag>,
            datalog::AndAnnotationPolicy<LiftedTag, datalog::MaxAggregation>,
            datalog::TerminationPolicy<LiftedTag, datalog::MaxAggregation>>(task,
                                                                            std::move(execution_context),
                                                                            datalog::OrAnnotationPolicy<LiftedTag>(),
                                                                            datalog::AndAnnotationPolicy<LiftedTag, datalog::MaxAggregation>(),
                                                                            cost_mode)
{
}

MaxRPGHeuristicPtr<LiftedTag> MaxRPGHeuristic<LiftedTag>::create(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode)
{
    return std::make_shared<MaxRPGHeuristic<LiftedTag>>(std::move(task), std::move(execution_context), cost_mode);
}

ygg::float_t MaxRPGHeuristic<LiftedTag>::extract_cost_and_set_preferred_actions_impl(const StateView<LiftedTag>& state)
{
    return m_workspace.tp.get_total_cost(datalog::FactSets { m_rpg_program.get_const_program_workspace().facts.fact_sets, m_workspace.facts.fact_sets },
                                         this->m_workspace.and_annot,
                                         this->m_workspace.numeric_and_annot,
                                         *this->m_workspace.numeric_support_selector);
}

}
