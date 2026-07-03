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
#include "tyr/datalog/lifted/policies/aggregation.hpp"
#include "tyr/datalog/policies/termination_concept.hpp"

#include <optional>

namespace tyr::datalog
{

template<>
class NoTerminationPolicy<GroundTag>
{
public:
    NoTerminationPolicy() = default;

    void set_goals(::tyr::formalism::datalog::GroundConjunctiveConditionView) {}
    bool check(::tyr::formalism::datalog::ProgramView<GroundTag>, const FactsWorkspace<GroundTag>&) const noexcept { return false; }
    Cost get_total_cost(const FactsWorkspace<GroundTag>&, const GroundSelectedPredicateAnnotations&) const noexcept { return Cost(0); }
    void reset() noexcept {}
    void clear() noexcept {}
};

template<typename AggregationFunction>
class TerminationPolicy<GroundTag, AggregationFunction>
{
public:
    void set_goals(::tyr::formalism::datalog::GroundConjunctiveConditionView goals_) { goals = goals_; }

    bool check(::tyr::formalism::datalog::ProgramView<GroundTag> program, const FactsWorkspace<GroundTag>& facts) const noexcept
    {
        if (!goals.has_value())
            return false;

        for (const auto literal : goals->template get_literals<::tyr::formalism::StaticTag>())
            if (literal.get_polarity() && !is_static_fact_true(program, literal.get_atom()))
                return false;

        for (const auto literal : goals->template get_literals<::tyr::formalism::FluentTag>())
            if (literal.get_polarity() && !facts.fluent_atoms.contains(literal.get_atom()))
                return false;

        return true;
    }

    Cost get_total_cost(const FactsWorkspace<GroundTag>&, const GroundSelectedPredicateAnnotations& and_annot) const noexcept
    {
        if (!goals.has_value())
            return Cost(0);

        auto total = AggregationFunction::identity();
        for (const auto literal : goals->template get_literals<::tyr::formalism::FluentTag>())
        {
            if (!literal.get_polarity())
                continue;
            if (const auto* annotation = and_annot.find(literal.get_atom()))
                total = agg(total, get_cost(*annotation));
        }
        return total;
    }

    const auto& get_goal() const noexcept { return goals; }

    void reset() noexcept {}
    void clear() noexcept { goals.reset(); }

private:
    static bool is_static_fact_true(::tyr::formalism::datalog::ProgramView<GroundTag> program,
                                    ::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::StaticTag> fact) noexcept
    {
        for (const auto atom : program.template get_atoms<::tyr::formalism::StaticTag>())
            if (atom.get_index() == fact.get_index())
                return true;
        return false;
    }

    std::optional<::tyr::formalism::datalog::GroundConjunctiveConditionView> goals;
    AggregationFunction agg;
};

static_assert(TerminationPolicyConcept<NoTerminationPolicy<GroundTag>, GroundTag>);
static_assert(TerminationPolicyConcept<TerminationPolicy<GroundTag, SumAggregation>, GroundTag>);
static_assert(TerminationPolicyConcept<TerminationPolicy<GroundTag, MaxAggregation>, GroundTag>);

}

#endif
