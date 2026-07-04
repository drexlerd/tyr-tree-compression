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

#ifndef TYR_DATALOG_GROUND_POLICIES_TERMINATION_HPP_
#define TYR_DATALOG_GROUND_POLICIES_TERMINATION_HPP_

#include "tyr/datalog/declarations.hpp"
#include "tyr/datalog/ground/policies/numeric_support.hpp"
#include "tyr/datalog/lifted/applicability.hpp"
#include "tyr/datalog/lifted/policies/aggregation.hpp"
#include "tyr/datalog/policies/termination_concept.hpp"

#include <cassert>
#include <limits>
#include <optional>
#include <yggdrasil/core/config.hpp>

namespace tyr::datalog
{

template<>
class NoTerminationPolicy<GroundTag>
{
public:
    NoTerminationPolicy() = default;

    void set_goals(::tyr::formalism::datalog::GroundConjunctiveConditionView) {}
    bool check(::tyr::formalism::datalog::ProgramView<GroundTag>, const FactsWorkspace<GroundTag>&) const noexcept { return false; }
    Cost get_total_cost(const FactsWorkspace<GroundTag>&,
                        const GroundSelectedPredicateAnnotations&,
                        const GroundSelectedFunctionAnnotations&,
                        const GroundNumericSupportSelector&) const noexcept
    {
        return Cost(0);
    }
    void reset() noexcept {}
    void clear() noexcept {}
};

template<typename AggregationFunction>
class TerminationPolicy<GroundTag, AggregationFunction>
{
public:
    void set_goals(::tyr::formalism::datalog::GroundConjunctiveConditionView goals_)
    {
        clear();
        goals = goals_;
    }

    bool check(::tyr::formalism::datalog::ProgramView<GroundTag>, const FactsWorkspace<GroundTag>& facts) const noexcept
    {
        if (!goals.has_value())
            return false;

        return is_applicable(*goals, FactSets { facts.static_fact_sets, facts.fluent_fact_sets });
    }

    Cost get_total_cost(const FactsWorkspace<GroundTag>&,
                        const GroundSelectedPredicateAnnotations& and_annot,
                        const GroundSelectedFunctionAnnotations&,
                        const GroundNumericSupportSelector& numeric_support_selector) const noexcept
    {
        if (!goals.has_value())
            return AggregationFunction::identity();

        auto total = AggregationFunction::identity();
        for (const auto literal : goals->template get_literals<::tyr::formalism::FluentTag>())
        {
            if (!literal.get_polarity())
                continue;

            const auto* annotation = and_annot.find(literal.get_atom());
            assert(annotation);
            if (!annotation)
                return std::numeric_limits<Cost>::max();

            const auto atom_cost = get_cost(*annotation);
            if (atom_cost == std::numeric_limits<Cost>::max())
                return std::numeric_limits<Cost>::max();
            total = agg(total, atom_cost);
        }

        for (const auto numeric_constraint : goals->get_numeric_constraints())
        {
            const auto constraint_cost = numeric_support_selector.get_constraint_cost(numeric_constraint, numeric_support_selector_workspace, agg);
            if (constraint_cost == std::numeric_limits<Cost>::max())
                return std::numeric_limits<Cost>::max();
            total = agg(total, constraint_cost);
        }

        return total;
    }

    const auto& get_goal() const noexcept { return goals; }

    void reset() noexcept {}

    void clear() noexcept
    {
        goals.reset();
        numeric_support_selector_workspace.clear();
    }

private:
    std::optional<::tyr::formalism::datalog::GroundConjunctiveConditionView> goals;
    mutable GroundNumericSupportSelectorWorkspace numeric_support_selector_workspace;
    AggregationFunction agg;
};

static_assert(TerminationPolicyConcept<NoTerminationPolicy<GroundTag>, GroundTag>);
static_assert(TerminationPolicyConcept<TerminationPolicy<GroundTag, SumAggregation>, GroundTag>);
static_assert(TerminationPolicyConcept<TerminationPolicy<GroundTag, MaxAggregation>, GroundTag>);

}

#endif
