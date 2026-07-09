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

#include "tyr/datalog/policies/annotation_types.hpp"
#include "tyr/formalism/datalog/repository.hpp"

namespace tyr::datalog
{

template<>
struct AndAnnotationContext<GroundTag>
{
    ygg::ClosedInterval<ygg::float_t> metric;
    Cost current_cost;
    std::vector<NumericSupport<GroundTag>> numeric_supports;
    ::tyr::formalism::datalog::GroundRuleView rule;
    const SelectedPredicateAnnotations<GroundTag>& program_and_annot;
};

using GroundWitnessAnnotation = WitnessAnnotation<GroundTag>;
using GroundBaseAnnotation = BaseAnnotation<GroundTag>;
using GroundAnnotation = Annotation<GroundTag>;
using GroundSelectedPredicateAnnotations = SelectedPredicateAnnotations<GroundTag>;
using GroundSelectedFunctionAnnotations = SelectedFunctionAnnotations<GroundTag>;
using GroundAndAnnotationContext = AndAnnotationContext<GroundTag>;
using GroundCostUpdate = CostUpdate<GroundTag>;

}

#endif
