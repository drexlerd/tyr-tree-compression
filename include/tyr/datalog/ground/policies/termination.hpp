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
#include "tyr/datalog/policies/aggregation.hpp"
#include "tyr/datalog/policies/termination_concept.hpp"

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
    void set_goals(::tyr::formalism::datalog::GroundConjunctiveConditionView goals_);

    bool check(::tyr::formalism::datalog::ProgramView<GroundTag>, const FactsWorkspace<GroundTag>& facts) const noexcept;

    Cost get_total_cost(const FactsWorkspace<GroundTag>&,
                        const GroundSelectedPredicateAnnotations& and_annot,
                        const GroundSelectedFunctionAnnotations&,
                        const GroundNumericSupportSelector& numeric_support_selector) const noexcept;

    const auto& get_goal() const noexcept { return goals; }

    void reset() noexcept;

    void clear() noexcept;

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
