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

#ifndef TYR_DATALOG_POLICIES_ANNOTATION_TYPES_HPP_
#define TYR_DATALOG_POLICIES_ANNOTATION_TYPES_HPP_

#include "tyr/datalog/declarations.hpp"
#include "tyr/datalog/lifted/policies/aggregation.hpp"

#include <cassert>
#include <limits>
#include <optional>
#include <variant>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>

namespace tyr::datalog
{

class NumericSupportSelector;
class NumericSupportSelectorWorkspace;

template<TaskKind Kind>
struct WitnessAnnotation;

template<TaskKind Kind>
struct BaseAnnotation
{
public:
    explicit BaseAnnotation(Cost cost = Cost(0)) : m_cost(cost) {}

    auto get_cost() const noexcept { return m_cost; }

private:
    Cost m_cost;
};

template<TaskKind Kind>
using Annotation = std::variant<BaseAnnotation<Kind>, WitnessAnnotation<Kind>>;

template<TaskKind Kind>
inline Cost get_cost(const Annotation<Kind>& annotation) noexcept
{
    return std::visit([](const auto& value) { return value.get_cost(); }, annotation);
}

template<TaskKind Kind>
class PredicateAnnotationMap;

template<TaskKind Kind>
using SelectedPredicateAnnotations = PredicateAnnotationMap<Kind>;

template<TaskKind Kind>
struct NumericIntervalAnnotation
{
    ygg::ClosedInterval<ygg::float_t> interval;
    Annotation<Kind> annotation;
};

template<TaskKind Kind>
class NumericIntervalAnnotations;

template<TaskKind Kind>
using SelectedFunctionAnnotations = NumericIntervalAnnotations<Kind>;

template<TaskKind Kind>
struct AndAnnotationContext;

template<TaskKind Kind>
struct CostUpdate
{
    std::optional<Cost> old_cost;
    Cost new_cost;

    CostUpdate() noexcept : old_cost(std::nullopt), new_cost(Cost(0)) { assert(is_monoton()); }
    CostUpdate(std::optional<Cost> old_cost, Cost new_cost) noexcept : old_cost(old_cost), new_cost(new_cost) { assert(is_monoton()); }
    CostUpdate(Cost old_cost, Cost new_cost) noexcept :
        old_cost(old_cost == std::numeric_limits<Cost>::max() ? std::nullopt : std::optional<Cost>(old_cost)),
        new_cost(new_cost)
    {
        assert(is_monoton());
    }

    bool is_monoton() const noexcept { return !old_cost || new_cost <= old_cost.value(); }
};

}

#endif
