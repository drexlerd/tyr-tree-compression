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

#include "tyr/planning/ground/heuristics/lmcut.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <ranges>

namespace f = tyr::formalism;

namespace tyr::planning
{

LMCutHeuristic<GroundTag>::LMCutHeuristic(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context) :
    Base(std::move(task),
         std::move(execution_context),
         datalog::OrAnnotationPolicy<GroundTag>(),
         datalog::AchieverAndAnnotationPolicy<GroundTag, datalog::MaxAggregation>(),
         datalog::TerminationPolicy<GroundTag, datalog::MaxAggregation>()),
    m_residual_costs(),
    m_goal_zone(),
    m_before_goal_zone(),
    m_not_before_goal_zone(),
    m_cut(),
    m_max_precondition_buffers(),
    m_max_precondition_depth(0)
{
}

LMCutHeuristicPtr<GroundTag> LMCutHeuristic<GroundTag>::create(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context)
{
    return std::make_shared<LMCutHeuristic<GroundTag>>(std::move(task), std::move(execution_context));
}

ygg::float_t LMCutHeuristic<GroundTag>::evaluate(const StateView<GroundTag>& state)
{
    auto value = datalog::Cost(0);
    m_residual_costs.clear();

    while (true)
    {
        apply_residual_costs();
        const auto hmax = Base::evaluate(state);
        if (hmax == std::numeric_limits<ygg::float_t>::infinity())
            return hmax;

        const auto hmax_cost = datalog::Cost(hmax);
        if (hmax_cost == 0)
            return ygg::float_t(value);

        extract_cut();
        if (m_cut.empty())
            return ygg::float_t(value + hmax_cost);

        auto cut_cost = std::numeric_limits<datalog::Cost>::max();
        for (const auto action : m_cut)
            cut_cost = std::min(cut_cost, get_residual_cost(action));

        assert(cut_cost > 0 && cut_cost != std::numeric_limits<datalog::Cost>::max());

        value += cut_cost;
        for (const auto action : m_cut)
            set_residual_cost(action, get_residual_cost(action) - cut_cost);
    }
}

ygg::float_t LMCutHeuristic<GroundTag>::extract_cost_and_set_preferred_actions_impl(const StateView<GroundTag>&) { return get_goal_cost(); }

datalog::Cost LMCutHeuristic<GroundTag>::get_residual_cost(Action action) const
{
    const auto it = m_residual_costs.find(action);
    return it == m_residual_costs.end() ? datalog::Cost(1) : it->second;
}

void LMCutHeuristic<GroundTag>::set_residual_cost(Action action, datalog::Cost cost) { m_residual_costs.insert_or_assign(action, cost); }

void LMCutHeuristic<GroundTag>::apply_residual_costs()
{
    m_workspace.clear_costs();
    const auto& mapping = m_task->get_rpg_program().get_ground_rule_to_action_mapping();
    for (const auto& [rule, action] : mapping)
        m_workspace.cost_policy.set_cost(rule, get_residual_cost(action));
}

const std::vector<LMCutHeuristic<GroundTag>::Atom>& LMCutHeuristic<GroundTag>::get_witness_max_preconditions(const datalog::GroundWitnessAnnotation& witness)
{
    if (m_max_precondition_depth == m_max_precondition_buffers.size())
        m_max_precondition_buffers.emplace_back();

    auto& result = m_max_precondition_buffers[m_max_precondition_depth++];
    result.clear();

    const auto rule = witness.get_rule();
    const auto& mapping = m_task->get_rpg_program().get_ground_rule_to_action_mapping();
    const auto it = mapping.find(rule);
    const auto rule_cost = it == mapping.end() ? datalog::Cost(0) : get_residual_cost(it->second);
    if (witness.get_cost() < rule_cost)
        return result;

    const auto body_cost = witness.get_cost() - rule_cost;
    for (const auto literal : rule.get_body().get_literals<f::FluentTag>())
        if (literal.get_polarity() && get_atom_cost(literal.get_atom()) == body_cost)
            result.push_back(literal.get_atom());
    return result;
}

void LMCutHeuristic<GroundTag>::release_witness_max_preconditions()
{
    assert(m_max_precondition_depth > 0);
    --m_max_precondition_depth;
}

void LMCutHeuristic<GroundTag>::mark_goal_zone(Atom atom)
{
    if (!m_goal_zone.insert(atom).second)
        return;

    const auto atom_cost = get_atom_cost(atom);
    if (const auto* achievers = m_workspace.and_ap.find_achievers(atom))
    {
        for (const auto& witness : *achievers)
        {
            if (witness.get_cost() != atom_cost)
                continue;

            const auto& mapping = m_task->get_rpg_program().get_ground_rule_to_action_mapping();
            const auto action_it = mapping.find(witness.get_rule());
            if (action_it != mapping.end() && get_residual_cost(action_it->second) > 0)
                continue;

            const auto& preconditions = get_witness_max_preconditions(witness);
            for (const auto precondition : preconditions)
                mark_goal_zone(precondition);
            release_witness_max_preconditions();
        }
    }
}

bool LMCutHeuristic<GroundTag>::is_before_goal_zone(Atom atom)
{
    if (m_goal_zone.contains(atom))
        return false;
    if (m_before_goal_zone.contains(atom))
        return true;
    if (m_not_before_goal_zone.contains(atom))
        return false;

    m_not_before_goal_zone.insert(atom);

    auto has_optimal_achiever = false;
    auto before = false;
    const auto atom_cost = get_atom_cost(atom);
    if (const auto* achievers = m_workspace.and_ap.find_achievers(atom))
    {
        for (const auto& witness : *achievers)
        {
            if (before || witness.get_cost() != atom_cost)
                continue;

            has_optimal_achiever = true;
            const auto& preconditions = get_witness_max_preconditions(witness);
            before = preconditions.empty() || std::ranges::any_of(preconditions, [&](const auto precondition) { return is_before_goal_zone(precondition); });
            release_witness_max_preconditions();
        }
    }

    if (!has_optimal_achiever)
        before = true;

    if (before)
    {
        m_not_before_goal_zone.erase(atom);
        m_before_goal_zone.insert(atom);
        return true;
    }

    return false;
}

void LMCutHeuristic<GroundTag>::extract_cut()
{
    m_goal_zone.clear();
    m_before_goal_zone.clear();
    m_not_before_goal_zone.clear();
    m_cut.clear();

    const auto goal_cost = get_goal_cost();
    if (const auto& goal = m_workspace.tp.get_goal())
    {
        for (const auto literal : goal->get_literals<f::FluentTag>())
        {
            if (literal.get_polarity() && get_atom_cost(literal.get_atom()) == goal_cost)
            {
                mark_goal_zone(literal.get_atom());
                break;
            }
        }
    }

    const auto& mapping = m_task->get_rpg_program().get_ground_rule_to_action_mapping();
    for (const auto atom : m_goal_zone)
    {
        const auto atom_cost = get_atom_cost(atom);
        if (const auto* achievers = m_workspace.and_ap.find_achievers(atom))
        {
            for (const auto& witness : *achievers)
            {
                if (witness.get_cost() != atom_cost)
                    continue;

                const auto action_it = mapping.find(witness.get_rule());
                if (action_it == mapping.end() || get_residual_cost(action_it->second) == 0)
                    continue;

                const auto& preconditions = get_witness_max_preconditions(witness);
                const auto crosses_cut =
                    preconditions.empty() || std::ranges::any_of(preconditions, [&](const auto precondition) { return is_before_goal_zone(precondition); });
                release_witness_max_preconditions();
                if (crosses_cut)
                    m_cut.insert(action_it->second);
            }
        }
    }
}

}
