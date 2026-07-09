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

#ifndef TYR_DATALOG_LIFTED_POLICIES_COST_HPP_
#define TYR_DATALOG_LIFTED_POLICIES_COST_HPP_

#include "tyr/datalog/declarations.hpp"
#include "tyr/datalog/lifted/policies/annotation_types.hpp"
#include "tyr/datalog/policies/cost.hpp"
#include "tyr/datalog/policies/cost_concept.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <span>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>

namespace tyr::datalog
{

template<>
class RuleCostOverridePolicy<LiftedTag> : public RuleCostOverrideStorage<LiftedTag>
{
public:
    using Base = RuleCostOverrideStorage<LiftedTag>;
    using RuleKey = Base::RuleKey;
    using NumericKey = Base::NumericKey;
    using CostMap = Base::CostMap;
    using NumericTransitionCostMap = Base::NumericTransitionCostMap;

    RuleCostOverridePolicy() = default;
    explicit RuleCostOverridePolicy(CostMap costs);

    Cost get_cost(RuleKey rule_binding) const;
    Cost get_cost(RuleKey rule_binding, NumericKey binding, ygg::ClosedInterval<ygg::float_t> interval) const;

    void clear() noexcept;

    using Base::set_cost;
    void set_prefix_cost(::tyr::formalism::datalog::RuleView rule, std::span<const ygg::Index<::tyr::formalism::Object>> objects, Cost cost);

    using Base::get_costs;
    using Base::get_numeric_transition_costs;

    size_t get_num_prefix_costs() const noexcept { return m_prefix_costs.size(); }

private:
    struct PrefixCost
    {
        ::tyr::formalism::datalog::RuleView rule;
        ygg::IndexList<::tyr::formalism::Object> objects;
        Cost cost;
    };

    CostMap::const_iterator find_override(::tyr::formalism::datalog::RuleBindingView rule_binding) const;

    NumericTransitionCostMap::const_iterator
    find_numeric_transition_override(::tyr::formalism::datalog::RuleBindingView rule_binding,
                                     ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> binding,
                                     ygg::ClosedInterval<ygg::float_t> interval) const;

    const Cost* find_prefix_override(::tyr::formalism::datalog::RuleBindingView rule_binding) const;

    std::vector<PrefixCost> m_prefix_costs;
};

static_assert(RuleCostPolicyConcept<RuleCostPolicy<LiftedTag>, LiftedTag>);
static_assert(RuleCostPolicyConcept<RuleCostOverridePolicy<LiftedTag>, LiftedTag>);
static_assert(MutableRuleCostPolicyConcept<RuleCostPolicy<LiftedTag>, LiftedTag>);
static_assert(MutableRuleCostPolicyConcept<RuleCostOverridePolicy<LiftedTag>, LiftedTag>);

}

#endif
