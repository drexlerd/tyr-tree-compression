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

#include <algorithm>
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
    explicit RuleCostOverridePolicy(CostMap costs) : Base(std::move(costs)), m_prefix_costs() {}

    Cost get_cost(RuleKey rule_binding) const
    {
        if (const auto it = find_override(rule_binding); it != m_costs.end())
            return it->second;
        if (const auto* cost = find_prefix_override(rule_binding))
            return *cost;
        return Cost(0);
    }

    Cost get_cost(RuleKey rule_binding, NumericKey binding, ygg::ClosedInterval<ygg::float_t> interval) const
    {
        if (const auto it = find_numeric_transition_override(rule_binding, binding, interval); it != m_numeric_transition_costs.end())
            return it->second;
        return Cost(0);
    }

    void clear() noexcept
    {
        Base::clear();
        m_prefix_costs.clear();
    }

    using Base::set_cost;

    void set_prefix_cost(::tyr::formalism::datalog::RuleView rule, std::span<const ygg::Index<::tyr::formalism::Object>> objects, Cost cost)
    {
        auto prefix_cost = PrefixCost { rule, ygg::IndexList<::tyr::formalism::Object>(objects.begin(), objects.end()), cost };
        for (auto& existing : m_prefix_costs)
        {
            if (existing.rule.get_index() == prefix_cost.rule.get_index() && existing.objects == prefix_cost.objects)
            {
                existing.cost = cost;
                return;
            }
        }
        m_prefix_costs.push_back(std::move(prefix_cost));
    }

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

    // Overrides are interned in the program workspace repository, while lifted rule workers query
    // bindings from overlay repositories. View equality includes repository identity, so fall back
    // to the relation and object values when the exact lookup misses.
    template<typename Binding>
    static bool equivalent_binding(Binding lhs, Binding rhs) noexcept
    {
        return lhs.get_relation().get_index() == rhs.get_relation().get_index() && lhs.get_data() == rhs.get_data();
    }

    CostMap::const_iterator find_override(::tyr::formalism::datalog::RuleBindingView rule_binding) const
    {
        if (const auto it = m_costs.find(rule_binding); it != m_costs.end())
            return it;

        for (auto it = m_costs.begin(); it != m_costs.end(); ++it)
        {
            if (equivalent_binding(it->first, rule_binding))
                return it;
        }

        return m_costs.end();
    }

    NumericTransitionCostMap::const_iterator
    find_numeric_transition_override(::tyr::formalism::datalog::RuleBindingView rule_binding,
                                     ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> binding,
                                     ygg::ClosedInterval<ygg::float_t> interval) const
    {
        const auto key = NumericTransitionCostKey<LiftedTag> { rule_binding, binding, interval };
        if (const auto it = m_numeric_transition_costs.find(key); it != m_numeric_transition_costs.end())
            return it;

        for (auto it = m_numeric_transition_costs.begin(); it != m_numeric_transition_costs.end(); ++it)
        {
            const auto& candidate = it->first;
            if (candidate.interval == interval && equivalent_binding(candidate.rule_key, rule_binding) && equivalent_binding(candidate.numeric_key, binding))
                return it;
        }

        return m_numeric_transition_costs.end();
    }

    const Cost* find_prefix_override(::tyr::formalism::datalog::RuleBindingView rule_binding) const
    {
        const auto objects = rule_binding.get_data();
        for (const auto& prefix_cost : m_prefix_costs)
        {
            if (prefix_cost.rule.get_index() == rule_binding.get_relation().get_index() && prefix_cost.objects.size() <= objects.size()
                && std::equal(prefix_cost.objects.begin(), prefix_cost.objects.end(), objects.begin()))
                return &prefix_cost.cost;
        }
        return nullptr;
    }

    std::vector<PrefixCost> m_prefix_costs;
};

static_assert(RuleCostPolicyConcept<RuleCostPolicy<LiftedTag>, LiftedTag>);
static_assert(RuleCostPolicyConcept<RuleCostOverridePolicy<LiftedTag>, LiftedTag>);
static_assert(MutableRuleCostPolicyConcept<RuleCostPolicy<LiftedTag>, LiftedTag>);
static_assert(MutableRuleCostPolicyConcept<RuleCostOverridePolicy<LiftedTag>, LiftedTag>);

}

#endif
