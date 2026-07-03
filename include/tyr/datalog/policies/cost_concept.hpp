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

#ifndef TYR_DATALOG_POLICIES_COST_CONCEPT_BY_KIND_HPP_
#define TYR_DATALOG_POLICIES_COST_CONCEPT_BY_KIND_HPP_

#include "tyr/datalog/ground/policies/cost_concept.hpp"
#include "tyr/datalog/lifted/policies/cost_concept.hpp"
#include "tyr/declarations.hpp"

namespace tyr::datalog
{

template<typename T, typename Kind>
concept RuleCostPolicyConcept = TaskKind<Kind> && details::RuleCostPolicyConceptImpl<Kind, T>::value;

template<typename T, typename Kind>
concept MutableRuleCostPolicyConcept = TaskKind<Kind> && details::MutableRuleCostPolicyConceptImpl<Kind, T>::value;

}

#endif
