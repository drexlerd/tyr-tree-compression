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

#ifndef TYR_DATALOG_LIFTED_POLICIES_ANNOTATION_TYPES_HPP_
#define TYR_DATALOG_LIFTED_POLICIES_ANNOTATION_TYPES_HPP_

#include "tyr/datalog/policies/annotation_types.hpp"
#include "tyr/formalism/datalog/repository.hpp"

namespace tyr::datalog
{

template<>
struct AndAnnotationContext<LiftedTag>
{
    Cost current_cost;
    std::vector<NumericSupport<LiftedTag>> numeric_supports;
    ::tyr::formalism::datalog::RuleView rule;
    ::tyr::formalism::datalog::RuleBindingView rule_binding;
    Cost metric_effect_cost;
    ::tyr::formalism::datalog::ConjunctiveConditionView witness_condition;
    const NumericSupportSelector<LiftedTag>& numeric_support_selector;
    NumericSupportSelectorWorkspace<LiftedTag>& numeric_support_selector_workspace;
    const SelectedPredicateAnnotations<LiftedTag>& program_and_annot;
    const SelectedFunctionAnnotations<LiftedTag>& program_numeric_and_annot;
    ::tyr::formalism::datalog::GrounderContext& delta_context;
    ::tyr::formalism::datalog::GrounderContext& iteration_context;
};

using LiftedWitnessAnnotation = WitnessAnnotation<LiftedTag>;
using LiftedBaseAnnotation = BaseAnnotation<LiftedTag>;
using LiftedAnnotation = Annotation<LiftedTag>;
using LiftedSelectedPredicateAnnotations = SelectedPredicateAnnotations<LiftedTag>;
using LiftedSelectedFunctionAnnotations = SelectedFunctionAnnotations<LiftedTag>;
using LiftedAndAnnotationContext = AndAnnotationContext<LiftedTag>;
using LiftedCostUpdate = CostUpdate<LiftedTag>;

}

#endif
