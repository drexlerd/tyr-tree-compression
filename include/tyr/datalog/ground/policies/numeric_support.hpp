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

#ifndef TYR_DATALOG_GROUND_POLICIES_NUMERIC_SUPPORT_HPP_
#define TYR_DATALOG_GROUND_POLICIES_NUMERIC_SUPPORT_HPP_

#include "tyr/datalog/ground/policies/annotation_types.hpp"
#include "tyr/datalog/ground/workspaces/facts.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <algorithm>
#include <limits>
#include <vector>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>

namespace tyr::datalog
{

class GroundNumericSupportSelectorWorkspace
{
public:
    struct SelectionEntry
    {
        ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term;
        ygg::ClosedInterval<ygg::float_t> interval;
        const NumericIntervalAnnotation<GroundTag>* annotation;
        Cost cost;

        bool operator<(const SelectionEntry& other) const noexcept { return cost < other.cost; }
    };

    void clear() noexcept;

    std::vector<SelectionEntry> selection;
};

class GroundNumericSupportSelector
{
public:
    GroundNumericSupportSelector(const FactsWorkspace<GroundTag>& facts,
                                 const NumericIntervalAnnotations<GroundTag>& annotations,
                                 bool initial_intervals_cost_zero = false);

    ygg::ClosedInterval<ygg::float_t> select_fluent_interval(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term,
                                                             std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection) const;

    ygg::ClosedInterval<ygg::float_t> evaluate_effect_expression(::tyr::formalism::datalog::GroundFunctionExpressionView expression,
                                                                 std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection) const;

    template<typename AggregationFunction>
    Cost get_constraint_cost(::tyr::formalism::datalog::GroundBooleanOperatorView constraint,
                             std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection,
                             AggregationFunction agg) const
    {
        return get_greedy_support_cost(selection, agg, [&](auto& selected) { return is_supported(constraint, selected); });
    }

    template<typename AggregationFunction>
    Cost get_constraint_cost(::tyr::formalism::datalog::GroundBooleanOperatorView constraint, AggregationFunction agg) const
    {
        m_selection.clear();
        return get_constraint_cost(constraint, m_selection, agg);
    }

    template<typename AggregationFunction>
    Cost get_constraint_cost(::tyr::formalism::datalog::GroundBooleanOperatorView constraint,
                             GroundNumericSupportSelectorWorkspace& workspace,
                             AggregationFunction agg) const
    {
        return get_constraint_cost(constraint, workspace.selection, agg);
    }

    template<typename AggregationFunction, typename Callback>
    Cost for_each_constraint_support(::tyr::formalism::datalog::GroundBooleanOperatorView constraint,
                                     GroundNumericSupportSelectorWorkspace& workspace,
                                     AggregationFunction agg,
                                     Callback callback) const
    {
        const auto cost = get_constraint_cost(constraint, workspace, agg);
        if (cost == std::numeric_limits<Cost>::max())
            return cost;

        for (const auto& entry : workspace.selection)
        {
            if (entry.annotation)
            {
                callback(entry.term, entry.interval, entry.annotation->annotation);
                continue;
            }

            const auto* entries = find_entries(entry.term);
            if (!entries)
                continue;

            for (const auto& candidate : *entries)
                if (is_available(entry.term, candidate.interval) && get_cost(candidate.annotation) <= entry.cost && subset(candidate.interval, entry.interval))
                    callback(entry.term, candidate.interval, candidate.annotation);
        }

        return cost;
    }

private:
    template<typename AggregationFunction, typename IsSupported>
    Cost get_greedy_support_cost(std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection,
                                 AggregationFunction agg,
                                 IsSupported is_supported) const
    {
        selection.clear();
        if (!is_supported(selection))
            return std::numeric_limits<Cost>::max();

        std::sort(selection.begin(), selection.end());
        for (size_t pos = 0; pos < selection.size(); ++pos)
        {
            const auto term = selection[pos].term;
            const auto* entries = find_entries(term);
            if (!entries)
                continue;

            const auto end = std::upper_bound(entries->begin(),
                                              entries->end(),
                                              selection[pos].cost,
                                              [](Cost cost, const auto& entry) { return cost < get_cost(entry.annotation); });

            for (auto it = entries->begin(); it != end; ++it)
            {
                if (!is_available(term, it->interval))
                    continue;

                const auto old_entry = selection[pos];
                selection[pos] = GroundNumericSupportSelectorWorkspace::SelectionEntry { term, it->interval, &*it, get_cost(it->annotation) };
                if (is_supported(selection))
                    break;
                selection[pos] = old_entry;
            }
        }

        auto cost = AggregationFunction::identity();
        for (const auto& entry : selection)
            cost = agg(cost, entry.cost);
        return cost;
    }

    bool is_supported(::tyr::formalism::datalog::GroundBooleanOperatorView constraint,
                      std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection) const;

    const NumericIntervalAnnotations<GroundTag>::Entries*
    find_entries(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term) const;
    ygg::ClosedInterval<ygg::float_t> current_interval(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term) const;
    Cost get_current_interval_cost(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term,
                                   ygg::ClosedInterval<ygg::float_t> current) const;
    bool is_available(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term, ygg::ClosedInterval<ygg::float_t> interval) const;
    GroundNumericSupportSelectorWorkspace::SelectionEntry*
    find_selection_entry(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term,
                         std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry>& selection) const;

    const FactsWorkspace<GroundTag>& m_facts;
    const NumericIntervalAnnotations<GroundTag>& m_annotations;
    bool m_initial_intervals_cost_zero;
    mutable std::vector<GroundNumericSupportSelectorWorkspace::SelectionEntry> m_selection;
};

}

#endif
