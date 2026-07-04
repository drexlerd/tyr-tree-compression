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
#include <type_traits>
#include <variant>

#include <yggdrasil/containers/variant.hpp>

namespace f = tyr::formalism;
namespace fd = tyr::formalism::datalog;

namespace tyr::planning
{
namespace
{

template<typename Head>
bool is_numeric_head(const Head&) noexcept
{
    return false;
}

bool is_numeric_head(fd::GroundNumericEffectOperatorView<f::FluentTag>) noexcept
{
    return true;
}

bool has_numeric_relaxation_content(fd::ProgramView<GroundTag> program)
{
    for (const auto rule : program.get_ground_rules())
    {
        if (!rule.get_body().get_numeric_constraints().empty())
            return true;

        if (ygg::visit([](auto&& head) { return is_numeric_head(head); }, rule.get_head()))
            return true;
    }

    return false;
}

}

LMCutHeuristic<GroundTag>::LMCutHeuristic(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode) :
    Base(std::move(task),
         std::move(execution_context),
         datalog::OrAnnotationPolicy<GroundTag>(),
         datalog::AchieverAndAnnotationPolicy<GroundTag, datalog::MaxAggregation>(),
         cost_mode),
    m_residual_costs(),
    m_goal_zone(),
    m_numeric_goal_zone(),
    m_before_goal_zone(),
    m_numeric_before_goal_zone(),
    m_not_before_goal_zone(),
    m_numeric_not_before_goal_zone(),
    m_cut(),
    m_max_precondition_buffers(),
    m_max_precondition_depth(0)
{
}

LMCutHeuristicPtr<GroundTag> LMCutHeuristic<GroundTag>::create(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode)
{
    return std::make_shared<LMCutHeuristic<GroundTag>>(std::move(task), std::move(execution_context), cost_mode);
}

ygg::float_t LMCutHeuristic<GroundTag>::evaluate(const StateView<GroundTag>& state) { return evaluate_impl(state); }

ygg::float_t LMCutHeuristic<GroundTag>::evaluate_impl(const StateView<GroundTag>& state)
{
    auto value = datalog::Cost(0);
    m_residual_costs.clear();

    while (true)
    {
        apply_residual_costs();
        const auto hmax = Base::evaluate_impl(state, false);
        if (hmax == std::numeric_limits<ygg::float_t>::infinity())
            return hmax;

        if (has_numeric_relaxation_content(m_rpg_program.get_datalog_program().get_program()))
            return hmax;

        const auto hmax_cost = datalog::Cost(hmax);
        if (hmax_cost == 0)
            return ygg::float_t(value);

        extract_cut();
        if (m_cut.empty())
            return ygg::float_t(value + hmax_cost);

        auto cut_cost = std::numeric_limits<datalog::Cost>::max();
        for (const auto key : m_cut)
            cut_cost = std::min(cut_cost, get_residual_cost(key));

        assert(cut_cost > 0 && cut_cost != std::numeric_limits<datalog::Cost>::max());

        value += cut_cost;
        for (const auto key : m_cut)
            set_residual_cost(key, get_residual_cost(key) - cut_cost);
    }
}

ygg::float_t LMCutHeuristic<GroundTag>::extract_cost_and_set_preferred_actions_impl(const StateView<GroundTag>&) { return get_goal_cost(); }

datalog::Cost LMCutHeuristic<GroundTag>::get_residual_cost(CostKey action_binding) const
{
    const auto used_it = m_residual_costs.find(action_binding);
    const auto used = used_it == m_residual_costs.end() ? datalog::Cost(0) : used_it->second;
    return used >= datalog::Cost(1) ? datalog::Cost(0) : datalog::Cost(1) - used;
}

datalog::Cost LMCutHeuristic<GroundTag>::get_residual_cost(Rule rule) const
{
    const auto action_it = m_rpg_program.get_rule_to_action_mapping().find(rule);
    return action_it == m_rpg_program.get_rule_to_action_mapping().end() ? datalog::Cost(0) : get_residual_cost(action_it->second.get_row());
}

void LMCutHeuristic<GroundTag>::set_residual_cost(CostKey action_binding, datalog::Cost cost)
{
    m_residual_costs.insert_or_assign(action_binding, datalog::Cost(1) - cost);
}

void LMCutHeuristic<GroundTag>::set_residual_cost(Rule rule, datalog::Cost cost)
{
    const auto action_it = m_rpg_program.get_rule_to_action_mapping().find(rule);
    if (action_it != m_rpg_program.get_rule_to_action_mapping().end())
        set_residual_cost(action_it->second.get_row(), cost);
}

void LMCutHeuristic<GroundTag>::apply_residual_costs()
{
    m_workspace.clear_costs();
    for (const auto& [action_binding, used_cost] : m_residual_costs)
        set_action_binding_cost(action_binding, used_cost);
}

datalog::Cost LMCutHeuristic<GroundTag>::get_numeric_cost(NumericNode node) const noexcept
{
    const auto* annotation = m_workspace.numeric_and_annot.find(node.term, node.interval);
    return annotation ? datalog::get_cost(*annotation) : datalog::Cost(0);
}

const datalog::GroundWitnessAnnotation* LMCutHeuristic<GroundTag>::get_numeric_witness(NumericNode node) const noexcept
{
    const auto* annotation = m_workspace.numeric_and_annot.find(node.term, node.interval);
    return annotation ? std::get_if<datalog::GroundWitnessAnnotation>(annotation) : nullptr;
}

const std::vector<LMCutHeuristic<GroundTag>::Precondition>&
LMCutHeuristic<GroundTag>::get_witness_max_preconditions(const datalog::GroundWitnessAnnotation& witness)
{
    if (m_max_precondition_depth == m_max_precondition_buffers.size())
        m_max_precondition_buffers.emplace_back();

    auto& result = m_max_precondition_buffers[m_max_precondition_depth++];
    result.clear();
    const auto rule = witness.get_rule();
    const auto rule_cost = get_residual_cost(rule);
    if (witness.get_cost() < rule_cost)
        return result;

    const auto body_cost = witness.get_cost() - rule_cost;
    for (const auto literal : rule.get_body().get_literals<f::FluentTag>())
        if (literal.get_polarity() && get_atom_cost(literal.get_atom()) == body_cost)
            result.emplace_back(literal.get_atom());

    for (const auto& support : witness.get_numeric_supports())
        if (support.get_cost() == body_cost)
            result.emplace_back(NumericNode { support.get_term(), support.get_interval() });

    return result;
}

void LMCutHeuristic<GroundTag>::release_witness_max_preconditions()
{
    assert(m_max_precondition_depth > 0);
    --m_max_precondition_depth;
}

void LMCutHeuristic<GroundTag>::mark_goal_zone(Precondition precondition)
{
    std::visit([&](auto node) { mark_goal_zone(node); }, precondition);
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

            const auto& mapping = m_rpg_program.get_rule_to_action_mapping();
            const auto action_it = mapping.find(witness.get_rule());
            if (action_it != mapping.end() && get_residual_cost(witness.get_rule()) > 0)
                continue;

            const auto& preconditions = get_witness_max_preconditions(witness);
            for (const auto& precondition : preconditions)
                mark_goal_zone(precondition);
            release_witness_max_preconditions();
        }
    }
}

void LMCutHeuristic<GroundTag>::mark_goal_zone(NumericNode node)
{
    if (!m_numeric_goal_zone.insert(node).second)
        return;

    const auto numeric_cost = get_numeric_cost(node);
    const auto* witness = get_numeric_witness(node);
    if (!witness || witness->get_cost() != numeric_cost)
        return;

    const auto action_it = m_rpg_program.get_rule_to_action_mapping().find(witness->get_rule());
    if (action_it != m_rpg_program.get_rule_to_action_mapping().end() && get_residual_cost(witness->get_rule()) > 0)
        return;

    const auto& preconditions = get_witness_max_preconditions(*witness);
    for (const auto& precondition : preconditions)
        mark_goal_zone(precondition);
    release_witness_max_preconditions();
}

bool LMCutHeuristic<GroundTag>::is_before_goal_zone(Precondition precondition)
{
    return std::visit([&](auto node) { return is_before_goal_zone(node); }, precondition);
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

bool LMCutHeuristic<GroundTag>::is_before_goal_zone(NumericNode node)
{
    if (m_numeric_goal_zone.contains(node))
        return false;
    if (m_numeric_before_goal_zone.contains(node))
        return true;
    if (m_numeric_not_before_goal_zone.contains(node))
        return false;

    m_numeric_not_before_goal_zone.insert(node);

    auto before = false;
    const auto* witness = get_numeric_witness(node);
    if (witness && witness->get_cost() == get_numeric_cost(node))
    {
        const auto& preconditions = get_witness_max_preconditions(*witness);
        before = preconditions.empty() || std::ranges::any_of(preconditions, [&](const auto precondition) { return is_before_goal_zone(precondition); });
        release_witness_max_preconditions();
    }
    else
    {
        before = true;
    }

    if (before)
    {
        m_numeric_not_before_goal_zone.erase(node);
        m_numeric_before_goal_zone.insert(node);
        return true;
    }

    return false;
}

void LMCutHeuristic<GroundTag>::extract_cut()
{
    m_goal_zone.clear();
    m_numeric_goal_zone.clear();
    m_before_goal_zone.clear();
    m_numeric_before_goal_zone.clear();
    m_not_before_goal_zone.clear();
    m_numeric_not_before_goal_zone.clear();
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

        auto selector = datalog::GroundNumericSupportSelector(m_workspace.facts, m_workspace.numeric_and_annot);
        auto selection = std::vector<datalog::GroundNumericSupportSelectorWorkspace::SelectionEntry> {};
        for (const auto numeric_constraint : goal->get_numeric_constraints())
        {
            if (selector.get_constraint_cost(numeric_constraint, selection, datalog::MaxAggregation {}) != goal_cost)
                continue;

            for (const auto& entry : selection)
                if (entry.cost == goal_cost)
                    mark_goal_zone(NumericNode { entry.term, entry.interval });
        }
    }

    auto inspect_witness = [&](const auto& witness)
    {
        const auto action_it = m_rpg_program.get_rule_to_action_mapping().find(witness.get_rule());
        if (action_it == m_rpg_program.get_rule_to_action_mapping().end() || get_residual_cost(action_it->second.get_row()) == 0)
            return;

        const auto& preconditions = get_witness_max_preconditions(witness);
        const auto crosses_cut = preconditions.empty()
                                 || std::ranges::any_of(preconditions, [&](const auto precondition) { return is_before_goal_zone(precondition); });
        release_witness_max_preconditions();
        if (crosses_cut)
            m_cut.insert(action_it->second.get_row());
    };

    for (const auto atom : m_goal_zone)
    {
        const auto atom_cost = get_atom_cost(atom);
        if (const auto* achievers = m_workspace.and_ap.find_achievers(atom))
            for (const auto& witness : *achievers)
                if (witness.get_cost() == atom_cost)
                    inspect_witness(witness);
    }

    for (const auto& node : m_numeric_goal_zone)
        if (const auto* witness = get_numeric_witness(node); witness && witness->get_cost() == get_numeric_cost(node))
            inspect_witness(*witness);
}

}
