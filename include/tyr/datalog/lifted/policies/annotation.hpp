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

#ifndef TYR_SOLVER_POLICIES_ANNOTATION_HPP_
#define TYR_SOLVER_POLICIES_ANNOTATION_HPP_

#include "tyr/datalog/declarations.hpp"
#include "tyr/datalog/lifted/policies/aggregation.hpp"
#include "tyr/datalog/lifted/policies/annotation_types.hpp"
#include "tyr/datalog/policies/annotation_concept.hpp"
#include "tyr/formalism/datalog/declarations.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <limits>
#include <optional>
#include <tuple>
#include <vector>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/config.hpp>

namespace tyr::datalog
{

class NumericSupportSelector;
class NumericSupportSelectorWorkspace;

template<>
class NoOrAnnotationPolicy<LiftedTag>
{
public:
    void initialize_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> head,
                               SelectedPredicateAnnotations& program_and_annot) const noexcept
    {
    }
    void initialize_annotation(::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> head,
                               ygg::ClosedInterval<ygg::float_t> interval,
                               SelectedFunctionAnnotations& program_numeric_and_annot) const noexcept
    {
    }

    CostUpdate update_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                                 ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                                 const SelectedPredicateAnnotations& delta_and_annot,
                                 SelectedPredicateAnnotations& program_and_annot) const noexcept
    {
        return CostUpdate();
    }
};

template<>
class NoAndAnnotationPolicy<LiftedTag>
{
public:
    void clear_achievers() noexcept {}

    void record_achiever(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                         const AndAnnotationContext& context) const noexcept
    {
    }

    void update_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                           ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                           const AndAnnotationContext& context,
                           SelectedPredicateAnnotations& delta_and_annot) const noexcept
    {
    }

    void update_annotation(::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> program_head,
                           ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> delta_head,
                           ygg::ClosedInterval<ygg::float_t> interval,
                           const AndAnnotationContext& context,
                           SelectedFunctionAnnotations& delta_numeric_and_annot) const
    {
    }
};

template<>
class OrAnnotationPolicy<LiftedTag>
{
public:
    void initialize_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                               SelectedPredicateAnnotations& program_and_annot) const;
    void initialize_annotation(::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> program_head,
                               ygg::ClosedInterval<ygg::float_t> interval,
                               SelectedFunctionAnnotations& program_numeric_and_annot) const;

    CostUpdate update_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                                 ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                                 const SelectedPredicateAnnotations& delta_and_annot,
                                 SelectedPredicateAnnotations& program_and_annot) const;
};

template<typename AggregationFunction>
class AndAnnotationPolicy<LiftedTag, AggregationFunction>
{
public:
    static constexpr AggregationFunction agg = AggregationFunction {};

    void clear_achievers() noexcept {}

    void record_achiever(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                         const AndAnnotationContext& context) const noexcept
    {
    }

    void update_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                           ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                           const AndAnnotationContext& context,
                           SelectedPredicateAnnotations& delta_and_annot) const;

    void update_annotation(::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> program_head,
                           ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> delta_head,
                           ygg::ClosedInterval<ygg::float_t> interval,
                           const AndAnnotationContext& context,
                           SelectedFunctionAnnotations& delta_numeric_and_annot) const;
};

template<typename AggregationFunction>
class AchieverAndAnnotationPolicy<LiftedTag, AggregationFunction> : public AndAnnotationPolicy<LiftedTag, AggregationFunction>
{
public:
    using PredicateBinding = ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag>;
    using PredicateBindingIndex = ygg::Index<::tyr::formalism::RelationBinding<::tyr::formalism::Predicate<::tyr::formalism::FluentTag>>>;
    using Achievers = std::vector<WitnessAnnotation>;

    void clear_achievers() noexcept;

    const Achievers* find_achievers(PredicateBinding program_head) const noexcept;

    void record_achiever(PredicateBinding program_head, const AndAnnotationContext& context) const;

private:
    mutable ygg::UnorderedMap<PredicateBindingIndex, Achievers> m_achievers;
};

}

#endif
