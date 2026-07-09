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

#ifndef TYR_DATALOG_POLICIES_TERMINATION_CONCEPT_HPP_
#define TYR_DATALOG_POLICIES_TERMINATION_CONCEPT_HPP_

#include "tyr/datalog/fact_sets.hpp"
#include "tyr/datalog/ground/policies/numeric_support.hpp"
#include "tyr/datalog/lifted/policies/numeric_support.hpp"
#include "tyr/datalog/policies/annotation_types.hpp"
#include "tyr/declarations.hpp"
#include "tyr/formalism/datalog/views.hpp"

#include <concepts>

namespace tyr::datalog::details
{

template<TaskKind Kind>
struct TerminationNumericSupportSelector;

template<>
struct TerminationNumericSupportSelector<GroundTag>
{
    using type = GroundNumericSupportSelector;
};

template<>
struct TerminationNumericSupportSelector<LiftedTag>
{
    using type = NumericSupportSelector;
};

template<TaskKind Kind>
using TerminationNumericSupportSelectorT = typename TerminationNumericSupportSelector<Kind>::type;


template<TaskKind Kind>
struct TerminationNumericSupportSelectorWorkspace;

template<>
struct TerminationNumericSupportSelectorWorkspace<GroundTag>
{
    using type = GroundNumericSupportSelectorWorkspace;
};

template<>
struct TerminationNumericSupportSelectorWorkspace<LiftedTag>
{
    using type = NumericSupportSelectorWorkspace;
};

template<TaskKind Kind>
using TerminationNumericSupportSelectorWorkspaceT = typename TerminationNumericSupportSelectorWorkspace<Kind>::type;

}

namespace tyr::datalog
{

template<typename T, typename Kind>
concept TerminationPolicyConcept = TaskKind<Kind> && requires(T& p,
                                                              const T& cp,
                                                              ::tyr::formalism::datalog::GroundConjunctiveConditionView goals,
                                                              const FactSets& fact_sets,
                                                              const SelectedPredicateAnnotations<Kind>& and_annot,
                                                              const SelectedFunctionAnnotations<Kind>& numeric_and_annot,
                                                              const details::TerminationNumericSupportSelectorT<Kind>& numeric_support_selector) {
    { p.set_goals(goals) } -> std::same_as<void>;
    { cp.check(fact_sets) } -> std::same_as<bool>;
    { cp.get_total_cost(fact_sets, and_annot, numeric_and_annot, numeric_support_selector) } -> std::same_as<Cost>;
    { p.reset() } -> std::same_as<void>;
    { p.clear() } -> std::same_as<void>;
};

}

#endif
