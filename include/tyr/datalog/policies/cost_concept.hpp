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

#ifndef TYR_DATALOG_POLICIES_COST_CONCEPT_HPP_
#define TYR_DATALOG_POLICIES_COST_CONCEPT_HPP_

#include "tyr/datalog/policies/annotation_types.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <concepts>

namespace tyr::datalog
{

template<typename T>
concept CostPolicyConcept = requires(const T& p, ::tyr::formalism::datalog::RuleView rule, ::tyr::formalism::datalog::RuleBindingView rule_binding) {
    { p.get_cost(rule, rule_binding) } -> std::same_as<Cost>;
};

template<typename T>
concept MutableCostPolicyConcept = CostPolicyConcept<T> && requires(T& p, ::tyr::formalism::datalog::RuleBindingView rule_binding, Cost cost) {
    { p.clear() } -> std::same_as<void>;
    { p.set_cost(rule_binding, cost) } -> std::same_as<void>;
};

}

#endif
