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

#include "tyr/planning/lifted/heuristics/rpg_add.hpp"

namespace tyr::planning
{
AddRPGHeuristic<LiftedTag>::AddRPGHeuristic(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context) :
    RPGBase<LiftedTag,
            AddRPGHeuristic<LiftedTag>,
            datalog::OrAnnotationPolicy<LiftedTag>,
            datalog::AndAnnotationPolicy<LiftedTag, datalog::SumAggregation>,
            datalog::TerminationPolicy<LiftedTag, datalog::SumAggregation>>(
        task,
        std::move(execution_context),
        datalog::OrAnnotationPolicy<LiftedTag>(),
        datalog::AndAnnotationPolicy<LiftedTag, datalog::SumAggregation>(),
        datalog::TerminationPolicy<LiftedTag, datalog::SumAggregation>(
            task->get_rpg_program().get_datalog_program().get_program().get_predicates<::tyr::formalism::FluentTag>(),
            task->get_rpg_program().get_datalog_program().get_workspace_repository()))
{
}

AddRPGHeuristicPtr<LiftedTag> AddRPGHeuristic<LiftedTag>::create(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context)
{
    return std::make_shared<AddRPGHeuristic<LiftedTag>>(std::move(task), std::move(execution_context));
}

ygg::float_t AddRPGHeuristic<LiftedTag>::extract_cost_and_set_preferred_actions_impl(const StateView<LiftedTag>& state)
{
    return m_workspace.tp.get_total_cost(
        datalog::FactSets { m_task->get_rpg_program().get_const_program_workspace().facts.fact_sets, m_workspace.facts.fact_sets },
        this->m_workspace.and_annot,
        this->m_workspace.numeric_and_annot,
        *this->m_workspace.numeric_support_selector);
}

}
