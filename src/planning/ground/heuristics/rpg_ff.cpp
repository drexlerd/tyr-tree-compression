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

#include "tyr/formalism/planning/formatter.hpp"
#include "tyr/planning/applicability.hpp"

#include <algorithm>
#include <cstdlib>
#include <fmt/core.h>
#include <utility>

namespace f = tyr::formalism;
namespace fd = tyr::formalism::datalog;
namespace fp = tyr::formalism::planning;

namespace tyr::planning
{
namespace
{
bool should_trace_hff_state(ygg::Index<State<GroundTag>> state_index)
{
    const auto* value = std::getenv("TYR_TRACE_HFF_STATE");
    if (!value)
        return false;

    char* end = nullptr;
    const auto requested = std::strtoull(value, &end, 10);
    return end != value && *end == '\0' && requested == state_index.get_value();
}

const char* cost_mode_name(CostMode cost_mode) noexcept
{
    switch (cost_mode)
    {
        case CostMode::UNIT: return "unit";
        case CostMode::GENERAL: return "general";
    }
    return "unknown";
}

void trace_relaxed_plan(ygg::Index<State<GroundTag>> state_index,
                        CostMode cost_mode,
                        ygg::float_t value,
                        const ygg::UnorderedSet<ygg::Index<fp::GroundAction>>& relaxed_plan,
                        const fp::Repository& repository)
{
    auto actions = std::vector<fp::GroundActionView> {};
    actions.reserve(relaxed_plan.size());
    for (const auto action_index : relaxed_plan)
        actions.push_back(ygg::make_view(action_index, repository));

    std::sort(actions.begin(), actions.end(), [](const auto lhs, const auto rhs) { return lhs.get_index() < rhs.get_index(); });

    fmt::print("[HFF] state={} cost_mode={} value={} relaxed_plan_size={}\n", state_index.get_value(), cost_mode_name(cost_mode), value, actions.size());
    for (const auto action : actions)
        fmt::print("[HFF]   action={} {}\n", action.get_index().get_value(), std::make_pair(action, fp::PlanFormatting()));
}
}

FFRPGHeuristic<GroundTag>::FFRPGHeuristic(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode) :
    RPGBase<GroundTag,
            FFRPGHeuristic<GroundTag>,
            datalog::OrAnnotationPolicy<GroundTag>,
            datalog::AndAnnotationPolicy<GroundTag, datalog::SumAggregation>,
            datalog::TerminationPolicy<GroundTag, datalog::SumAggregation>,
            datalog::RuleCostOverridePolicy<GroundTag>>(task,
                                                        std::move(execution_context),
                                                        datalog::OrAnnotationPolicy<GroundTag>(),
                                                        datalog::AndAnnotationPolicy<GroundTag, datalog::SumAggregation>(),
                                                        cost_mode),
    m_markings(1),
    m_function_markings(),
    m_numeric_support_selector_workspace(),
    m_effect_families(),
    m_relaxed_plan(),
    m_preferred_actions(),
    m_preferred_action_views(),
    m_preferred_action_views_dirty(true)
{
    m_markings.front().resize(m_rpg_program.get_datalog_program().get_program().get_atoms<f::FluentTag>().size());
}

FFRPGHeuristicPtr<GroundTag> FFRPGHeuristic<GroundTag>::create(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context, CostMode cost_mode)
{
    return std::make_shared<FFRPGHeuristic<GroundTag>>(std::move(task), std::move(execution_context), cost_mode);
}

ygg::float_t FFRPGHeuristic<GroundTag>::extract_cost_and_set_preferred_actions_impl(const StateView<GroundTag>& state)
{
    m_preferred_action_views_dirty = true;
    m_relaxed_plan.clear();
    m_preferred_actions.clear();
    m_function_markings.clear();
    m_numeric_support_selector_workspace.clear();
    for (auto& bitset : m_markings)
        bitset.reset();

    const auto state_context = StateContext<GroundTag>(*this->m_task, state.get_unpacked_state(), ygg::float_t(0));
    if (const auto& goal = this->m_workspace.tp.get_goal())
    {
        for (const auto literal : goal->get_literals<f::FluentTag>())
            if (literal.get_polarity())
                extract_relaxed_plan_and_preferred_actions(literal.get_atom(), state_context);

        for (const auto constraint : goal->get_numeric_constraints())
            extract_numeric_constraint_support(constraint, state_context);
    }

    const auto value = ygg::float_t(m_relaxed_plan.size());
    if (should_trace_hff_state(state.get_index()))
        trace_relaxed_plan(state.get_index(), this->m_cost_mode, value, m_relaxed_plan, *this->m_task->get_repository());

    return value;
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

bool FFRPGHeuristic<GroundTag>::mark_function(fd::GroundFunctionTermView<f::FluentTag> term) { return !m_function_markings.insert(term).second; }

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

void FFRPGHeuristic<GroundTag>::extract_relaxed_plan_and_preferred_actions(fd::GroundFunctionTermView<f::FluentTag> term,
                                                                           const StateContext<GroundTag>& state_context)
{
    const auto* annotation = this->m_workspace.numeric_and_annot.find(term);
    if (!annotation)
        return;

    extract_relaxed_plan_and_preferred_actions(term, *annotation, state_context);
}

void FFRPGHeuristic<GroundTag>::extract_relaxed_plan_and_preferred_actions(fd::GroundFunctionTermView<f::FluentTag> term,
                                                                           const datalog::GroundAnnotation& annotation,
                                                                           const StateContext<GroundTag>& state_context)
{
    if (mark_function(term))
        return;

    const auto* witness = std::get_if<datalog::GroundWitnessAnnotation>(&annotation);
    if (!witness)
        return;

    extract_relaxed_plan_and_preferred_actions(*witness, state_context);
}

void FFRPGHeuristic<GroundTag>::extract_numeric_constraint_support(fd::GroundBooleanOperatorView constraint, const StateContext<GroundTag>& state_context)
{
    const auto numeric_support_selector = datalog::GroundNumericSupportSelector(this->m_workspace.facts, this->m_workspace.numeric_and_annot);
    numeric_support_selector.for_each_constraint_support(constraint,
                                                         m_numeric_support_selector_workspace,
                                                         datalog::SumAggregation {},
                                                         [&](const auto term, const auto, const auto& annotation)
                                                         { extract_relaxed_plan_and_preferred_actions(term, annotation, state_context); });
}

void FFRPGHeuristic<GroundTag>::extract_relaxed_plan_and_preferred_actions(const datalog::GroundWitnessAnnotation& witness,
                                                                           const StateContext<GroundTag>& state_context)
{
    const auto rule = witness.get_rule();
    const auto& mapping = this->m_rpg_program.get_rule_to_action_mapping();
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

    for (const auto constraint : rule.get_body().get_numeric_constraints())
        extract_numeric_constraint_support(constraint, state_context);
}

}
