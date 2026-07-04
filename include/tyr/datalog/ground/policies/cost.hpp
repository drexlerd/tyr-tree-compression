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

#ifndef TYR_DATALOG_GROUND_POLICIES_COST_HPP_
#define TYR_DATALOG_GROUND_POLICIES_COST_HPP_

#include "tyr/datalog/declarations.hpp"
#include "tyr/datalog/policies/cost_concept.hpp"

#include <tuple>
#include <utility>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/containers/associative_containers.hpp>

namespace tyr::datalog
{

struct GroundNumericTransitionCostKey
{
    ::tyr::formalism::datalog::GroundRuleView rule;
    ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term;
    ygg::ClosedInterval<ygg::float_t> interval;

    auto identifying_members() const noexcept { return std::make_tuple(rule, term, lower(interval), upper(interval)); }
};

template<>
class RuleCostPolicy<GroundTag>
{
public:
    Cost get_cost(::tyr::formalism::datalog::GroundRuleView) const noexcept { return Cost(0); }
    Cost get_cost(::tyr::formalism::datalog::GroundRuleView,
                  ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>,
                  ygg::ClosedInterval<ygg::float_t>) const noexcept
    {
        return Cost(0);
    }
    void clear() noexcept {}
    void set_cost(::tyr::formalism::datalog::GroundRuleView, Cost) noexcept {}
    void set_cost(::tyr::formalism::datalog::GroundRuleView,
                  ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>,
                  ygg::ClosedInterval<ygg::float_t>,
                  Cost) noexcept
    {
    }
};

template<>
class RuleCostOverridePolicy<GroundTag>
{
public:
    using CostMap = ygg::UnorderedMap<::tyr::formalism::datalog::GroundRuleView, Cost>;
    using NumericTransitionCostMap = ygg::UnorderedMap<GroundNumericTransitionCostKey, Cost>;

    RuleCostOverridePolicy() = default;
    explicit RuleCostOverridePolicy(CostMap costs_) : costs(std::move(costs_)), numeric_transition_costs() {}

    Cost get_cost(::tyr::formalism::datalog::GroundRuleView rule) const
    {
        if (const auto it = costs.find(rule); it != costs.end())
            return it->second;
        return Cost(0);
    }

    Cost get_cost(::tyr::formalism::datalog::GroundRuleView rule,
                  ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term,
                  ygg::ClosedInterval<ygg::float_t> interval) const
    {
        if (const auto it = numeric_transition_costs.find(GroundNumericTransitionCostKey { rule, term, interval }); it != numeric_transition_costs.end())
            return it->second;
        return Cost(0);
    }

    void clear() noexcept
    {
        costs.clear();
        numeric_transition_costs.clear();
    }

    void set_cost(::tyr::formalism::datalog::GroundRuleView rule, Cost cost) { costs.insert_or_assign(rule, cost); }

    void set_cost(::tyr::formalism::datalog::GroundRuleView rule,
                  ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term,
                  ygg::ClosedInterval<ygg::float_t> interval,
                  Cost cost)
    {
        numeric_transition_costs.insert_or_assign(GroundNumericTransitionCostKey { rule, term, interval }, cost);
    }

    const CostMap& get_costs() const noexcept { return costs; }
    CostMap& get_costs() noexcept { return costs; }
    const NumericTransitionCostMap& get_numeric_transition_costs() const noexcept { return numeric_transition_costs; }

private:
    CostMap costs;
    NumericTransitionCostMap numeric_transition_costs;
};

static_assert(RuleCostPolicyConcept<RuleCostPolicy<GroundTag>, GroundTag>);
static_assert(RuleCostPolicyConcept<RuleCostOverridePolicy<GroundTag>, GroundTag>);
static_assert(MutableRuleCostPolicyConcept<RuleCostPolicy<GroundTag>, GroundTag>);
static_assert(MutableRuleCostPolicyConcept<RuleCostOverridePolicy<GroundTag>, GroundTag>);

}

#endif
