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

#ifndef TYR_SOLVER_POLICIES_ANNOTATION_CONCEPT_HPP_
#define TYR_SOLVER_POLICIES_ANNOTATION_CONCEPT_HPP_

#include "tyr/datalog/lifted/policies/aggregation.hpp"
#include "tyr/datalog/lifted/policies/annotation_types.hpp"
#include "tyr/declarations.hpp"
#include "tyr/formalism/datalog/declarations.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <concepts>
#include <type_traits>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>

namespace tyr::datalog::details
{

template<typename Kind, typename T>
struct OrAnnotationPolicyConceptImpl : std::false_type
{
};

template<typename Kind, typename T>
struct AndAnnotationPolicyConceptImpl : std::false_type
{
};

template<typename T>
struct OrAnnotationPolicyConceptImpl<LiftedTag, T>
{
    static constexpr bool value = requires(const T& p,
                                           ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                                           ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> program_function_head,
                                           ygg::ClosedInterval<ygg::float_t> interval,
                                           ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                                           const SelectedPredicateAnnotations<LiftedTag>& delta_and_annot,
                                           SelectedPredicateAnnotations<LiftedTag>& program_and_annot,
                                           SelectedFunctionAnnotations<LiftedTag>& program_numeric_and_annot) {
        { p.initialize_annotation(program_head, program_and_annot) } -> std::same_as<void>;
        { p.initialize_annotation(program_function_head, interval, program_numeric_and_annot) } -> std::same_as<void>;
        { p.update_annotation(program_head, delta_head, delta_and_annot, program_and_annot) } -> std::same_as<CostUpdate<LiftedTag>>;
    };
};

template<typename T>
struct AndAnnotationPolicyConceptImpl<LiftedTag, T>
{
    static constexpr bool value = requires(const T& p,
                                           ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                                           ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                                           const AndAnnotationContext<LiftedTag>& context,
                                           SelectedPredicateAnnotations<LiftedTag>& delta_and_annot) {
        { p.update_annotation(program_head, delta_head, context, delta_and_annot) } -> std::same_as<void>;
    };
};

}

#endif
