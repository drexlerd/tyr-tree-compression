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

#ifndef TYR_DATALOG_POLICIES_COST_HPP_
#define TYR_DATALOG_POLICIES_COST_HPP_

#include "tyr/datalog/declarations.hpp"
#include "tyr/datalog/lifted/policies/annotation_types.hpp"
#include "tyr/datalog/policies/cost_concept.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <span>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/containers/vector.hpp>

namespace tyr::datalog
{

template<>
class RuleCostPolicy<LiftedTag>
{
public:
    Cost get_cost(::tyr::formalism::datalog::RuleView, ::tyr::formalism::datalog::RuleBindingView) const noexcept { return Cost(0); }
    void clear() noexcept {}
    void set_cost(::tyr::formalism::datalog::RuleBindingView, Cost) noexcept {}
};

template<>
class RuleCostOverridePolicy<LiftedTag>
{
public:
    using CostMap = ygg::UnorderedMap<::tyr::formalism::datalog::RuleBindingView, Cost>;

    RuleCostOverridePolicy() = default;
    explicit RuleCostOverridePolicy(CostMap costs) : m_costs(std::move(costs)), m_prefix_costs() {}

    Cost get_cost(::tyr::formalism::datalog::RuleView rule, ::tyr::formalism::datalog::RuleBindingView rule_binding) const
    {
        if (const auto it = find_override(rule_binding); it != m_costs.end())
            return it->second;
        if (const auto* cost = find_prefix_override(rule_binding))
            return *cost;
        return Cost(0);
    }

    void clear() noexcept
    {
        m_costs.clear();
        m_prefix_costs.clear();
    }

    void set_cost(::tyr::formalism::datalog::RuleBindingView rule_binding, Cost cost) { m_costs.insert_or_assign(rule_binding, cost); }

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

    const CostMap& get_costs() const noexcept { return m_costs; }
    CostMap& get_costs() noexcept { return m_costs; }
    size_t get_num_prefix_costs() const noexcept { return m_prefix_costs.size(); }

private:
    struct PrefixCost
    {
        ::tyr::formalism::datalog::RuleView rule;
        ygg::IndexList<::tyr::formalism::Object> objects;
        Cost cost;
    };

    CostMap::const_iterator find_override(::tyr::formalism::datalog::RuleBindingView rule_binding) const
    {
        if (const auto it = m_costs.find(rule_binding); it != m_costs.end())
            return it;

        for (auto it = m_costs.begin(); it != m_costs.end(); ++it)
        {
            const auto candidate = it->first;
            if (candidate.get_relation().get_index() != rule_binding.get_relation().get_index())
                continue;
            if (candidate.get_objects().get_data() == rule_binding.get_objects().get_data())
                return it;
        }

        return m_costs.end();
    }

    const Cost* find_prefix_override(::tyr::formalism::datalog::RuleBindingView rule_binding) const
    {
        const auto objects = rule_binding.get_data();
        for (const auto& prefix_cost : m_prefix_costs)
        {
            if (prefix_cost.rule.get_index() != rule_binding.get_relation().get_index())
                continue;
            if (prefix_cost.objects.size() > objects.size())
                continue;

            auto matches = true;
            for (size_t i = 0; i < prefix_cost.objects.size(); ++i)
            {
                if (prefix_cost.objects[i] != objects[i])
                {
                    matches = false;
                    break;
                }
            }
            if (matches)
                return &prefix_cost.cost;
        }
        return nullptr;
    }

    CostMap m_costs;
    std::vector<PrefixCost> m_prefix_costs;
};

static_assert(RuleCostPolicyConcept<RuleCostPolicy<LiftedTag>, LiftedTag>);
static_assert(RuleCostPolicyConcept<RuleCostOverridePolicy<LiftedTag>, LiftedTag>);
static_assert(MutableRuleCostPolicyConcept<RuleCostPolicy<LiftedTag>, LiftedTag>);
static_assert(MutableRuleCostPolicyConcept<RuleCostOverridePolicy<LiftedTag>, LiftedTag>);

}

#endif
