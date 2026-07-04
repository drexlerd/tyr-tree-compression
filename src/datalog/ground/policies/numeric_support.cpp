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

#include "tyr/datalog/ground/policies/numeric_support.hpp"

#include "tyr/formalism/arithmetic_operator_utils.hpp"
#include "tyr/formalism/boolean_operator_utils.hpp"

#include <limits>
#include <numeric>

namespace fd = tyr::formalism::datalog;
namespace f = tyr::formalism;

namespace tyr::datalog
{
namespace
{
ygg::ClosedInterval<ygg::float_t> evaluate(ygg::float_t value,
                                           const FactsWorkspace<GroundTag>&,
                                           const GroundNumericSupportSelector&,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>&)
{
    return ygg::ClosedInterval<ygg::float_t>(value, value);
}

ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundFunctionTermView<f::StaticTag> term,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundNumericSupportSelector&,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>&)
{
    const auto it = facts.static_fterm_intervals.find(term);
    return it == facts.static_fterm_intervals.end() ? ygg::ClosedInterval<ygg::float_t>() : it->second;
}

ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundFunctionTermView<f::FluentTag> term,
                                           const FactsWorkspace<GroundTag>&,
                                           const GroundNumericSupportSelector& selector,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection)
{
    return selector.select_fluent_interval(term, selection);
}

ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundFunctionExpressionView expression,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundNumericSupportSelector& selector,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection);

ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundArithmeticOperatorView expression,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundNumericSupportSelector& selector,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection);

template<f::ArithmeticOpKind O>
ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundUnaryOperatorView<O> expression,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundNumericSupportSelector& selector,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection)
{
    return f::apply(O {}, evaluate(expression.get_arg(), facts, selector, selection));
}

template<f::ArithmeticOpKind O>
ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundBinaryOperatorView<O> expression,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundNumericSupportSelector& selector,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection)
{
    return f::apply(O {}, evaluate(expression.get_lhs(), facts, selector, selection), evaluate(expression.get_rhs(), facts, selector, selection));
}

template<f::ArithmeticOpKind O>
ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundMultiOperatorView<O> expression,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundNumericSupportSelector& selector,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection)
{
    const auto args = expression.get_args();
    if (args.empty())
        return ygg::ClosedInterval<ygg::float_t>();
    return std::accumulate(std::next(args.begin()),
                           args.end(),
                           evaluate(args.front(), facts, selector, selection),
                           [&](const auto& value, const auto& arg) { return f::apply(O {}, value, evaluate(arg, facts, selector, selection)); });
}

template<f::BooleanOpKind O>
bool evaluate(fd::GroundBinaryOperatorView<O> expression,
              const FactsWorkspace<GroundTag>& facts,
              const GroundNumericSupportSelector& selector,
              std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection)
{
    return f::apply_existential(O {}, evaluate(expression.get_lhs(), facts, selector, selection), evaluate(expression.get_rhs(), facts, selector, selection));
}

ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundFunctionExpressionView expression,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundNumericSupportSelector& selector,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection)
{
    return ygg::visit([&](auto&& arg) { return evaluate(arg, facts, selector, selection); }, expression.get_variant());
}

ygg::ClosedInterval<ygg::float_t> evaluate(fd::GroundArithmeticOperatorView expression,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundNumericSupportSelector& selector,
                                           std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection)
{
    return ygg::visit([&](auto&& arg) { return evaluate(arg, facts, selector, selection); }, expression.get_variant());
}

bool evaluate(fd::GroundBooleanOperatorView expression,
              const FactsWorkspace<GroundTag>& facts,
              const GroundNumericSupportSelector& selector,
              std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection)
{
    return ygg::visit([&](auto&& arg) { return evaluate(arg, facts, selector, selection); }, expression.get_variant());
}
}

void GroundNumericSupportSelectorWorkspace::clear() noexcept { selection.clear(); }

GroundNumericSupportSelector::GroundNumericSupportSelector(const FactsWorkspace<GroundTag>& facts,
                                                           const NumericIntervalAnnotations<GroundTag>& annotations,
                                                           bool initial_intervals_cost_zero) :
    m_facts(facts),
    m_annotations(annotations),
    m_initial_intervals_cost_zero(initial_intervals_cost_zero),
    m_selection()
{
}

bool GroundNumericSupportSelector::is_supported(fd::GroundBooleanOperatorView constraint,
                                                std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection) const
{
    return evaluate(constraint, m_facts, *this, selection);
}

ygg::ClosedInterval<ygg::float_t>
GroundNumericSupportSelector::evaluate_effect_expression(fd::GroundFunctionExpressionView expression,
                                                         std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection) const
{
    return evaluate(expression, m_facts, *this, selection);
}

ygg::ClosedInterval<ygg::float_t>
GroundNumericSupportSelector::select_fluent_interval(fd::GroundFunctionTermView<f::FluentTag> term,
                                                     std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection) const
{
    if (const auto* entry = find_selection_entry(term, selection))
        return entry->interval;

    const auto current = current_interval(term);
    if (empty(current))
        return ygg::ClosedInterval<ygg::float_t>();

    const auto cost = get_current_interval_cost(term, current);
    if (cost == std::numeric_limits<Cost>::max())
        return ygg::ClosedInterval<ygg::float_t>();

    selection.push_back(GroundNumericSupportSelectorWorkspace::SelectionEntry { term, current, nullptr, cost });
    return current;
}

const NumericIntervalAnnotations<GroundTag>::Entries* GroundNumericSupportSelector::find_entries(fd::GroundFunctionTermView<f::FluentTag> term) const
{
    const auto relation_it = m_annotations.partitions().find(term.get_function());
    if (relation_it == m_annotations.partitions().end())
        return nullptr;

    const auto term_it = relation_it->second.find(term.get_index());
    return term_it == relation_it->second.end() ? nullptr : &term_it->second;
}

ygg::ClosedInterval<ygg::float_t> GroundNumericSupportSelector::current_interval(fd::GroundFunctionTermView<f::FluentTag> term) const
{
    const auto it = m_facts.fluent_fterm_intervals.find(term);
    return it == m_facts.fluent_fterm_intervals.end() ? ygg::ClosedInterval<ygg::float_t>() : it->second;
}

Cost GroundNumericSupportSelector::get_current_interval_cost(fd::GroundFunctionTermView<f::FluentTag> term, ygg::ClosedInterval<ygg::float_t> current) const
{
    const auto* entries = find_entries(term);
    if (!entries)
        return m_initial_intervals_cost_zero ? Cost(0) : std::numeric_limits<Cost>::max();

    auto best_cost = std::numeric_limits<Cost>::max();
    auto covered = ygg::ClosedInterval<ygg::float_t>();
    for (auto it = entries->begin(); it != entries->end();)
    {
        const auto candidate_cost = get_cost(it->annotation);
        const auto end = std::upper_bound(it, entries->end(), candidate_cost, [](Cost cost, const auto& entry) { return cost < get_cost(entry.annotation); });

        for (; it != end; ++it)
            if (is_available(term, it->interval))
                covered = empty(covered) ? it->interval : hull(covered, it->interval);

        if (!empty(covered) && subset(current, covered))
        {
            best_cost = candidate_cost;
            break;
        }
    }
    return best_cost;
}

bool GroundNumericSupportSelector::is_available(fd::GroundFunctionTermView<f::FluentTag> term, ygg::ClosedInterval<ygg::float_t> interval) const
{
    const auto current = current_interval(term);
    return !empty(current) && subset(interval, current);
}

GroundNumericSupportSelectorWorkspace::SelectionEntry*
GroundNumericSupportSelector::find_selection_entry(fd::GroundFunctionTermView<f::FluentTag> term,
                                                   std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection) const
{
    for (auto& entry : selection)
        if (entry.term.get_index() == term.get_index())
            return &entry;
    return nullptr;
}

}
