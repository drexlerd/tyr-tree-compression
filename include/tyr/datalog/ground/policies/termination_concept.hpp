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

#ifndef TYR_DATALOG_GROUND_POLICIES_TERMINATION_CONCEPT_HPP_
#define TYR_DATALOG_GROUND_POLICIES_TERMINATION_CONCEPT_HPP_

#include "tyr/datalog/ground/policies/annotation_types.hpp"
#include "tyr/datalog/ground/workspaces/facts.hpp"
#include "tyr/declarations.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <concepts>

namespace tyr::datalog::details
{

template<typename Kind, typename T>
struct TerminationPolicyConceptImpl;

template<typename T>
struct TerminationPolicyConceptImpl<GroundTag, T>
{
    static constexpr bool value = requires(T& p,
                                           const T& cp,
                                           ::tyr::formalism::datalog::GroundConjunctiveConditionView goals,
                                           ::tyr::formalism::datalog::GroundProgramView program,
                                           const FactsWorkspace<GroundTag>& facts,
                                           const GroundSelectedPredicateAnnotations& and_annot) {
        { p.set_goals(goals) } -> std::same_as<void>;
        { cp.check(program, facts) } -> std::same_as<bool>;
        { cp.get_total_cost(facts, and_annot) } -> std::same_as<Cost>;
        { p.reset() } -> std::same_as<void>;
        { p.clear() } -> std::same_as<void>;
    };
};

}

#endif
