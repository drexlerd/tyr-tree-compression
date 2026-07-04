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

#include "tyr/planning/lifted/heuristics/lmcut.hpp"

#include <algorithm>
#include <limits>
#include <ranges>
#include <type_traits>
#include <variant>

#include <yggdrasil/containers/variant.hpp>

namespace tyr::planning
{
namespace
{
namespace f = ::tyr::formalism;
namespace fd = ::tyr::formalism::datalog;

template<typename Head>
bool is_numeric_head(const Head&) noexcept
{
    return false;
}

bool is_numeric_head(fd::NumericEffectOperatorView<f::FluentTag>) noexcept
{
    return true;
}

bool has_numeric_relaxation_content(fd::ProgramView<LiftedTag> program)
{
    for (const auto rule : program.get_rules())
    {
        if (!rule.get_body().get_numeric_constraints().empty())
            return true;

        if (ygg::visit([](auto&& head) { return is_numeric_head(head); }, rule.get_head()))
            return true;
    }

    return false;
}

}

LMCutHeuristic<LiftedTag>::LMCutHeuristic(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode) :
    Base(task,
         std::move(execution_context),
         datalog::OrAnnotationPolicy<LiftedTag>(),
         datalog::AchieverAndAnnotationPolicy<LiftedTag, datalog::MaxAggregation>(),
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

LMCutHeuristicPtr<LiftedTag> LMCutHeuristic<LiftedTag>::create(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode)
{
    return std::make_shared<LMCutHeuristic<LiftedTag>>(std::move(task), std::move(execution_context), cost_mode);
}

ygg::float_t LMCutHeuristic<LiftedTag>::evaluate(const StateView<LiftedTag>& state)
{
    auto value = datalog::Cost(0);
    m_residual_costs.clear();

    while (true)
    {
        apply_residual_costs();
        const auto hmax = Base::evaluate(state);
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
        for (const auto action_binding : m_cut)
            cut_cost = std::min(cut_cost, get_residual_cost(action_binding));

        assert(cut_cost > 0 && cut_cost != std::numeric_limits<datalog::Cost>::max());

        value += cut_cost;
        for (const auto action_binding : m_cut)
            set_residual_cost(action_binding, get_residual_cost(action_binding) - cut_cost);
    }
}

ygg::float_t LMCutHeuristic<LiftedTag>::extract_cost_and_set_preferred_actions_impl(const StateView<LiftedTag>&) { return get_goal_cost(); }

datalog::Cost LMCutHeuristic<LiftedTag>::get_residual_cost(ActionBinding action_binding) const
{
    const auto it = m_residual_costs.find(action_binding);
    return it == m_residual_costs.end() ? datalog::Cost(1) : it->second;
}

void LMCutHeuristic<LiftedTag>::set_residual_cost(ActionBinding action_binding, datalog::Cost cost) { m_residual_costs.insert_or_assign(action_binding, cost); }

void LMCutHeuristic<LiftedTag>::apply_residual_costs()
{
    m_workspace.clear_costs();
    for (const auto& [action_binding, cost] : m_residual_costs)
        set_action_binding_cost(action_binding, datalog::Cost(1) - cost);
}

datalog::Cost LMCutHeuristic<LiftedTag>::get_numeric_cost(NumericNode node) const noexcept
{
    const auto* annotation = m_workspace.numeric_and_annot.find(node.binding, node.interval);
    return annotation ? datalog::get_cost(*annotation) : datalog::Cost(0);
}

const datalog::WitnessAnnotation<LiftedTag>* LMCutHeuristic<LiftedTag>::get_numeric_witness(NumericNode node) const noexcept
{
    const auto* annotation = m_workspace.numeric_and_annot.find(node.binding, node.interval);
    return annotation ? std::get_if<datalog::WitnessAnnotation<LiftedTag>>(annotation) : nullptr;
}

const std::vector<LMCutHeuristic<LiftedTag>::Precondition>&
LMCutHeuristic<LiftedTag>::get_witness_max_preconditions(const datalog::WitnessAnnotation<LiftedTag>& witness)
{
    if (m_max_precondition_depth == m_max_precondition_buffers.size())
        m_max_precondition_buffers.emplace_back();

    auto& result = m_max_precondition_buffers[m_max_precondition_depth++];
    result.clear();

    const auto action_binding = get_action_binding(witness);
    const auto rule_cost = action_binding ? get_residual_cost(*action_binding) : datalog::Cost(0);
    if (witness.get_cost() < rule_cost)
        return result;

    const auto body_cost = witness.get_cost() - rule_cost;
    for_each_witness_precondition(witness,
                                  [&](const auto precondition)
                                  {
                                      if (get_binding_cost(precondition) == body_cost)
                                          result.emplace_back(precondition);
                                  });

    for (const auto& support : witness.get_numeric_supports())
        if (support.get_cost() == body_cost)
            result.emplace_back(NumericNode { support.get_binding(), support.get_interval() });

    return result;
}

void LMCutHeuristic<LiftedTag>::release_witness_max_preconditions()
{
    assert(m_max_precondition_depth > 0);
    --m_max_precondition_depth;
}

void LMCutHeuristic<LiftedTag>::mark_goal_zone(Precondition precondition)
{
    std::visit([&](auto node) { mark_goal_zone(node); }, precondition);
}

void LMCutHeuristic<LiftedTag>::mark_goal_zone(PredicateBinding binding)
{
    if (!m_goal_zone.insert(binding).second)
        return;

    const auto binding_cost = get_binding_cost(binding);
    for_each_achiever(binding,
                      [&](const auto& witness)
                      {
                          if (witness.get_cost() != binding_cost)
                              return;

                          const auto action_binding = get_action_binding(witness);
                          if (action_binding && get_residual_cost(*action_binding) > 0)
                              return;

                          const auto& preconditions = get_witness_max_preconditions(witness);
                          for (const auto& precondition : preconditions)
                              mark_goal_zone(precondition);
                          release_witness_max_preconditions();
                      });
}

void LMCutHeuristic<LiftedTag>::mark_goal_zone(NumericNode node)
{
    if (!m_numeric_goal_zone.insert(node).second)
        return;

    const auto* witness = get_numeric_witness(node);
    if (!witness || witness->get_cost() != get_numeric_cost(node))
        return;

    const auto action_binding = get_action_binding(*witness);
    if (action_binding && get_residual_cost(*action_binding) > 0)
        return;

    const auto& preconditions = get_witness_max_preconditions(*witness);
    for (const auto& precondition : preconditions)
        mark_goal_zone(precondition);
    release_witness_max_preconditions();
}

bool LMCutHeuristic<LiftedTag>::is_before_goal_zone(Precondition precondition)
{
    return std::visit([&](auto node) { return is_before_goal_zone(node); }, precondition);
}

bool LMCutHeuristic<LiftedTag>::is_before_goal_zone(PredicateBinding binding)
{
    if (m_goal_zone.contains(binding))
        return false;
    if (m_before_goal_zone.contains(binding))
        return true;
    if (m_not_before_goal_zone.contains(binding))
        return false;

    m_not_before_goal_zone.insert(binding);

    auto has_optimal_achiever = false;
    auto before = false;
    const auto binding_cost = get_binding_cost(binding);
    for_each_achiever(binding,
                      [&](const auto& witness)
                      {
                          if (before || witness.get_cost() != binding_cost)
                              return;

                          has_optimal_achiever = true;

                          const auto& preconditions = get_witness_max_preconditions(witness);
                          before = preconditions.empty()
                                   || std::ranges::any_of(preconditions, [&](const auto precondition) { return is_before_goal_zone(precondition); });
                          release_witness_max_preconditions();
                      });

    if (!has_optimal_achiever)
        before = true;

    if (before)
    {
        m_not_before_goal_zone.erase(binding);
        m_before_goal_zone.insert(binding);
        return true;
    }

    return false;
}

bool LMCutHeuristic<LiftedTag>::is_before_goal_zone(NumericNode node)
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

void LMCutHeuristic<LiftedTag>::extract_cut()
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
        for (const auto literal : goal->get_literals<::tyr::formalism::FluentTag>())
        {
            if (literal.get_polarity() && get_binding_cost(literal.get_atom().get_row()) == goal_cost)
            {
                mark_goal_zone(literal.get_atom().get_row());
                break;
            }
        }

        auto selection_workspace = datalog::NumericSupportSelectorWorkspace {};
        for (const auto constraint : goal->get_numeric_constraints())
        {
            if (m_workspace.numeric_support_selector->get_constraint_cost(constraint, selection_workspace, datalog::MaxAggregation {}) != goal_cost)
                continue;

            for (const auto& entry : selection_workspace.selection)
                if (entry.cost == goal_cost)
                    mark_goal_zone(NumericNode { entry.binding, entry.interval });
        }
    }

    auto inspect_witness = [&](const auto& witness)
    {
        const auto action_binding = get_action_binding(witness);
        if (!action_binding || get_residual_cost(*action_binding) == 0)
            return;

        const auto& preconditions = get_witness_max_preconditions(witness);
        const auto crosses_cut = preconditions.empty()
                                 || std::ranges::any_of(preconditions, [&](const auto precondition) { return is_before_goal_zone(precondition); });
        release_witness_max_preconditions();
        if (crosses_cut)
            m_cut.insert(*action_binding);
    };

    for (const auto binding : m_goal_zone)
    {
        const auto binding_cost = get_binding_cost(binding);
        for_each_achiever(binding,
                          [&](const auto& witness)
                          {
                              if (witness.get_cost() == binding_cost)
                                  inspect_witness(witness);
                          });
    }

    for (const auto& node : m_numeric_goal_zone)
        if (const auto* witness = get_numeric_witness(node); witness && witness->get_cost() == get_numeric_cost(node))
            inspect_witness(*witness);
}

}
