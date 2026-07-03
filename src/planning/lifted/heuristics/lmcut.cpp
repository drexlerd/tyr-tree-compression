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

#include <limits>

namespace tyr::planning
{

LMCutHeuristic<LiftedTag>::LMCutHeuristic(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context) :
    Base(task,
         std::move(execution_context),
         datalog::OrAnnotationPolicy<LiftedTag>(),
         datalog::AchieverAndAnnotationPolicy<LiftedTag, datalog::MaxAggregation>(),
         datalog::TerminationPolicy<LiftedTag, datalog::MaxAggregation>(
             task->get_rpg_program().get_datalog_program().get_program().get_predicates<::tyr::formalism::FluentTag>(),
             task->get_rpg_program().get_datalog_program().get_workspace_repository())),
    m_residual_costs(),
    m_goal_zone(),
    m_before_goal_zone(),
    m_not_before_goal_zone(),
    m_cut(),
    m_max_precondition_buffers(),
    m_max_precondition_depth(0)
{
}

LMCutHeuristicPtr<LiftedTag> LMCutHeuristic<LiftedTag>::create(TaskPtr<LiftedTag> task, ygg::ExecutionContextPtr execution_context)
{
    return std::make_shared<LMCutHeuristic<LiftedTag>>(std::move(task), std::move(execution_context));
}

ygg::float_t LMCutHeuristic<LiftedTag>::evaluate(const StateView<LiftedTag>& state)
{
    const auto& program = m_task->get_rpg_program().get_datalog_program().get_program();
    if (!program.get_functions<::tyr::formalism::FluentTag>().empty())
        return Base::evaluate(state);

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
        set_action_binding_cost(action_binding, cost);
}

const std::vector<LMCutHeuristic<LiftedTag>::PredicateBinding>&
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
                                          result.push_back(precondition);
                                  });
    return result;
}

void LMCutHeuristic<LiftedTag>::release_witness_max_preconditions()
{
    assert(m_max_precondition_depth > 0);
    --m_max_precondition_depth;
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
                          for (const auto precondition : preconditions)
                              mark_goal_zone(precondition);
                          release_witness_max_preconditions();
                      });
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

void LMCutHeuristic<LiftedTag>::extract_cut()
{
    m_goal_zone.clear();
    m_before_goal_zone.clear();
    m_not_before_goal_zone.clear();
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
    }

    for (const auto binding : m_goal_zone)
    {
        const auto binding_cost = get_binding_cost(binding);
        for_each_achiever(binding,
                          [&](const auto& witness)
                          {
                              if (witness.get_cost() != binding_cost)
                                  return;

                              const auto action_binding = get_action_binding(witness);
                              if (!action_binding || get_residual_cost(*action_binding) == 0)
                                  return;

                              const auto& preconditions = get_witness_max_preconditions(witness);
                              const auto crosses_cut =
                                  preconditions.empty()
                                  || std::ranges::any_of(preconditions, [&](const auto precondition) { return is_before_goal_zone(precondition); });
                              release_witness_max_preconditions();
                              if (crosses_cut)
                                  m_cut.insert(*action_binding);
                          });
    }
}

}
