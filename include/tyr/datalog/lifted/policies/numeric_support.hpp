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

#ifndef TYR_SOLVER_POLICIES_NUMERIC_SUPPORT_HPP_
#define TYR_SOLVER_POLICIES_NUMERIC_SUPPORT_HPP_

#include "tyr/datalog/fact_sets.hpp"
#include "tyr/datalog/lifted/policies/annotation_types.hpp"
#include "tyr/datalog/policies/numeric_support_core.hpp"
#include "tyr/formalism/datalog/repository.hpp"
#include "tyr/formalism/datalog/views.hpp"

#include <limits>
#include <vector>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/semantics/equal_to.hpp>

namespace tyr::datalog
{
class NumericSupportSelectorWorkspace
{
public:
    struct SelectionEntry
    {
        ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> binding;
        ygg::ClosedInterval<ygg::float_t> interval;
        const NumericIntervalAnnotation<LiftedTag>* annotation;
        Cost cost;

        bool operator<(const SelectionEntry& other) const noexcept { return cost < other.cost; }
    };

    void clear() noexcept;

    std::vector<SelectionEntry> selection;
};

class NumericSupportSelector : public NumericSupportSelectorCore<NumericSupportSelector,
                                                                 ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag>,
                                                                 NumericSupportSelectorWorkspace::SelectionEntry>
{
public:
    using Key = ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag>;
    using SelectionEntry = NumericSupportSelectorWorkspace::SelectionEntry;
    using Core = NumericSupportSelectorCore<NumericSupportSelector, Key, SelectionEntry>;

    NumericSupportSelector(const FactSets& fact_sets, const NumericIntervalAnnotations<LiftedTag>& annotations);

    /**
     * Storage accessors for the shared core.
     */

    static Key key_of(const SelectionEntry& entry) noexcept { return entry.binding; }
    Key fluent_key(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term) const noexcept { return term.get_row(); }
    ygg::ClosedInterval<ygg::float_t> lookup_static(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::StaticTag> term) const;
    ygg::ClosedInterval<ygg::float_t> current_interval(Key key) const;
    const NumericIntervalAnnotations<LiftedTag>::Entries* find_entries(Key key) const;
    bool keys_equal(Key lhs, Key rhs) const noexcept { return m_binding_equal(lhs, rhs); }
    Cost missing_entries_cost() const noexcept { return std::numeric_limits<Cost>::max(); }

    /**
     * Workspace-based conveniences on top of the shared core.
     */

    using Core::get_constraint_cost;
    using Core::for_each_constraint_support;

    template<typename AggregationFunction>
    Cost get_constraint_cost(::tyr::formalism::datalog::GroundBooleanOperatorView constraint,
                             NumericSupportSelectorWorkspace& workspace,
                             AggregationFunction agg) const
    {
        return get_constraint_cost(constraint, workspace.selection, agg);
    }

    template<typename AggregationFunction, typename Callback>
    Cost for_each_constraint_support(::tyr::formalism::datalog::GroundBooleanOperatorView constraint,
                                     NumericSupportSelectorWorkspace& workspace,
                                     AggregationFunction agg,
                                     Callback callback) const
    {
        return for_each_constraint_support(constraint, workspace.selection, agg, callback);
    }

private:
    FactSets m_fact_sets;
    const NumericIntervalAnnotations<LiftedTag>& m_annotations;
    ygg::EqualTo<Key> m_binding_equal;
};
}

#endif
