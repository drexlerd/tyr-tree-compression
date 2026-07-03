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

#include "tyr/planning/ground/heuristics/rpg_ff.hpp"

#include "tyr/planning/applicability.hpp"

namespace f = tyr::formalism;
namespace fd = tyr::formalism::datalog;

namespace tyr::planning
{

FFRPGHeuristic<GroundTag>::FFRPGHeuristic(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context) :
    RPGBase<GroundTag,
            FFRPGHeuristic<GroundTag>,
            datalog::OrAnnotationPolicy<GroundTag>,
            datalog::AndAnnotationPolicy<GroundTag, datalog::SumAggregation>,
            datalog::TerminationPolicy<GroundTag, datalog::SumAggregation>>(task,
                                                                            std::move(execution_context),
                                                                            datalog::OrAnnotationPolicy<GroundTag>(),
                                                                            datalog::AndAnnotationPolicy<GroundTag, datalog::SumAggregation>(),
                                                                            datalog::TerminationPolicy<GroundTag, datalog::SumAggregation>()),
    m_markings(1),
    m_effect_families(),
    m_relaxed_plan(),
    m_preferred_actions(),
    m_preferred_action_views(),
    m_preferred_action_views_dirty(true)
{
    m_markings.front().resize(task->get_rpg_program().get_datalog_program().get_program().get_atoms<f::FluentTag>().size());
}

FFRPGHeuristicPtr<GroundTag> FFRPGHeuristic<GroundTag>::create(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context)
{
    return std::make_shared<FFRPGHeuristic<GroundTag>>(std::move(task), std::move(execution_context));
}

ygg::float_t FFRPGHeuristic<GroundTag>::extract_cost_and_set_preferred_actions_impl(const StateView<GroundTag>& state)
{
    m_preferred_action_views_dirty = true;
    m_relaxed_plan.clear();
    m_preferred_actions.clear();
    for (auto& bitset : m_markings)
        bitset.reset();

    const auto state_context = StateContext<GroundTag>(*this->m_task, state.get_unpacked_state(), ygg::float_t(0));
    if (const auto& goal = this->m_workspace.tp.get_goal())
    {
        for (const auto literal : goal->get_literals<f::FluentTag>())
            if (literal.get_polarity())
                extract_relaxed_plan_and_preferred_actions(literal.get_atom(), state_context);
    }

    return m_relaxed_plan.size();
}

const ygg::UnorderedSet<ygg::Index<::tyr::formalism::planning::GroundAction>>& FFRPGHeuristic<GroundTag>::get_preferred_actions()
{
    return m_preferred_actions;
}

const ygg::UnorderedSet<::tyr::formalism::planning::GroundActionView>& FFRPGHeuristic<GroundTag>::get_preferred_action_views()
{
    if (m_preferred_action_views_dirty)
    {
        m_preferred_action_views_dirty = false;
        m_preferred_action_views.clear();
        const auto& repository = *this->m_task->get_repository();
        for (const auto action_index : m_preferred_actions)
            m_preferred_action_views.insert(ygg::make_view(action_index, repository));
    }
    return m_preferred_action_views;
}

bool FFRPGHeuristic<GroundTag>::mark_atom(fd::GroundAtomView<f::FluentTag> atom)
{
    const auto i = ygg::uint_t(atom.get_index());
    if (m_markings.front().size() <= i)
        m_markings.front().resize(i + 1);
    if (ygg::test(i, m_markings.front()))
        return true;
    ygg::set(i, true, m_markings.front());
    return false;
}

void FFRPGHeuristic<GroundTag>::extract_relaxed_plan_and_preferred_actions(fd::GroundAtomView<f::FluentTag> atom, const StateContext<GroundTag>& state_context)
{
    if (mark_atom(atom))
        return;

    const auto* annotation = this->m_workspace.and_annot.find(atom);
    if (!annotation)
        return;

    const auto* witness = std::get_if<datalog::GroundWitnessAnnotation>(annotation);
    if (!witness)
        return;

    extract_relaxed_plan_and_preferred_actions(*witness, state_context);
}

void FFRPGHeuristic<GroundTag>::extract_relaxed_plan_and_preferred_actions(const datalog::GroundWitnessAnnotation& witness,
                                                                           const StateContext<GroundTag>& state_context)
{
    const auto rule = witness.get_rule();
    const auto& mapping = this->m_task->get_rpg_program().get_rule_to_action_mapping();
    if (const auto it = mapping.find(rule); it != mapping.end())
    {
        const auto action = it->second;
        m_relaxed_plan.insert(action.get_index());
        m_effect_families.clear();
        if (is_applicable(action, state_context, m_effect_families))
            m_preferred_actions.insert(action.get_index());
    }

    for (const auto literal : rule.get_body().get_literals<f::FluentTag>())
    {
        if (literal.get_polarity())
            extract_relaxed_plan_and_preferred_actions(literal.get_atom(), state_context);
    }
}

}
