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

#ifndef TYR_DATALOG_GROUND_POLICIES_ANNOTATION_CONCEPT_HPP_
#define TYR_DATALOG_GROUND_POLICIES_ANNOTATION_CONCEPT_HPP_

#include "tyr/datalog/ground/policies/annotation_types.hpp"
#include "tyr/declarations.hpp"

#include <concepts>
#include <type_traits>

namespace tyr::datalog::details
{

template<typename Kind, typename T>
struct OrAnnotationPolicyConceptImpl;

template<typename Kind, typename T>
struct AndAnnotationPolicyConceptImpl;

template<typename T>
struct OrAnnotationPolicyConceptImpl<GroundTag, T>
{
    static constexpr bool value = requires(const T& p,
                                           ::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                                           const GroundAnnotation& delta_and_annot,
                                           GroundSelectedPredicateAnnotations& program_and_annot) {
        { p.initialize_annotation(program_head, program_and_annot) } -> std::same_as<void>;
        { p.update_annotation(program_head, delta_and_annot, program_and_annot) } -> std::same_as<GroundCostUpdate>;
    };
};

template<typename T>
struct AndAnnotationPolicyConceptImpl<GroundTag, T>
{
    static constexpr bool value = requires(const T& p,
                                           ::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                                           const GroundAndAnnotationContext& context,
                                           GroundSelectedPredicateAnnotations& delta_and_annot) {
        { p.update_annotation(program_head, context, delta_and_annot) } -> std::same_as<void>;
    };
};

}

#endif
