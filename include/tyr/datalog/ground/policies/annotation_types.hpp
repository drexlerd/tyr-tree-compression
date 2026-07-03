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

#ifndef TYR_DATALOG_GROUND_POLICIES_ANNOTATION_TYPES_HPP_
#define TYR_DATALOG_GROUND_POLICIES_ANNOTATION_TYPES_HPP_

#include "tyr/datalog/lifted/policies/aggregation.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <cassert>
#include <limits>
#include <optional>
#include <tuple>
#include <utility>
#include <variant>
#include <yggdrasil/containers/associative_containers.hpp>

namespace tyr::datalog
{

struct GroundWitnessAnnotation
{
    GroundWitnessAnnotation(::tyr::formalism::datalog::GroundRuleView rule_, Cost cost_) : rule(rule_), cost(cost_) {}

    auto get_rule() const noexcept { return rule; }
    auto get_cost() const noexcept { return cost; }

    auto identifying_members() const noexcept { return std::tie(rule); }

private:
    ::tyr::formalism::datalog::GroundRuleView rule;
    Cost cost;
};

struct GroundBaseAnnotation
{
    explicit GroundBaseAnnotation(Cost cost_ = Cost(0)) : cost(cost_) {}

    auto get_cost() const noexcept { return cost; }

private:
    Cost cost;
};

using GroundAnnotation = std::variant<GroundBaseAnnotation, GroundWitnessAnnotation>;

inline Cost get_cost(const GroundAnnotation& annotation) noexcept
{
    return std::visit([](const auto& value) { return value.get_cost(); }, annotation);
}

class GroundAnnotationMap
{
public:
    using Atom = ::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>;

    void clear() noexcept { annotations.clear(); }

    void insert_or_assign(Atom atom, GroundAnnotation annotation) { annotations.insert_or_assign(atom, std::move(annotation)); }

    const GroundAnnotation* find(Atom atom) const noexcept
    {
        const auto it = annotations.find(atom);
        return it == annotations.end() ? nullptr : &it->second;
    }

    GroundAnnotation* find(Atom atom) noexcept
    {
        const auto it = annotations.find(atom);
        return it == annotations.end() ? nullptr : &it->second;
    }

private:
    ygg::UnorderedMap<Atom, GroundAnnotation> annotations;
};

using GroundSelectedPredicateAnnotations = GroundAnnotationMap;

struct GroundAndAnnotationContext
{
    Cost current_cost;
    ::tyr::formalism::datalog::GroundRuleView rule;
    Cost rule_cost;
    const GroundSelectedPredicateAnnotations& program_and_annot;
};

struct GroundCostUpdate
{
    std::optional<Cost> old_cost;
    Cost new_cost;

    GroundCostUpdate() noexcept : old_cost(std::nullopt), new_cost(Cost(0)) { assert(is_monoton()); }
    GroundCostUpdate(std::optional<Cost> old_cost_, Cost new_cost_) noexcept : old_cost(old_cost_), new_cost(new_cost_) { assert(is_monoton()); }
    GroundCostUpdate(Cost old_cost_, Cost new_cost_) noexcept :
        old_cost(old_cost_ == std::numeric_limits<Cost>::max() ? std::nullopt : std::optional<Cost>(old_cost_)),
        new_cost(new_cost_)
    {
        assert(is_monoton());
    }

    bool is_monoton() const noexcept { return !old_cost || new_cost <= old_cost.value(); }
};

}

#endif
