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

#ifndef TYR_DATALOG_GROUND_POLICIES_ANNOTATION_HPP_
#define TYR_DATALOG_GROUND_POLICIES_ANNOTATION_HPP_

#include "tyr/datalog/declarations.hpp"
#include "tyr/datalog/ground/policies/annotation_types.hpp"
#include "tyr/datalog/policies/aggregation.hpp"
#include "tyr/datalog/policies/annotation_concept.hpp"

#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>

namespace tyr::datalog
{

template<>
class NoOrAnnotationPolicy<GroundTag>
{
public:
    void initialize_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>, GroundSelectedPredicateAnnotations&) const noexcept;

    void initialize_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>,
                               ygg::ClosedInterval<ygg::float_t>,
                               GroundSelectedFunctionAnnotations&) const noexcept;

    GroundCostUpdate update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                                       const GroundAnnotation&,
                                       GroundSelectedPredicateAnnotations&) const noexcept;
};

template<>
class NoAndAnnotationPolicy<GroundTag>
{
public:
    void clear_achievers() noexcept;

    void record_achiever(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>, const GroundAndAnnotationContext&) const noexcept;

    void update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                           const GroundAndAnnotationContext&,
                           GroundSelectedPredicateAnnotations&) const noexcept;

    void update_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>,
                           ygg::ClosedInterval<ygg::float_t>,
                           const GroundAndAnnotationContext&,
                           GroundSelectedFunctionAnnotations&) const noexcept;
};

template<>
class OrAnnotationPolicy<GroundTag>
{
public:
    void initialize_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                               GroundSelectedPredicateAnnotations& program_and_annot) const;

    void initialize_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> program_head,
                               ygg::ClosedInterval<ygg::float_t> interval,
                               GroundSelectedFunctionAnnotations& program_numeric_and_annot) const;

    GroundCostUpdate update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                                       const GroundAnnotation& delta_and_annot,
                                       GroundSelectedPredicateAnnotations& program_and_annot) const;
};

template<typename AggregationFunction>
class AndAnnotationPolicy<GroundTag, AggregationFunction>
{
public:
    static constexpr AggregationFunction agg = AggregationFunction {};

    void clear_achievers() noexcept;

    void record_achiever(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>, const GroundAndAnnotationContext&) const noexcept;

    void update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                           const GroundAndAnnotationContext& context,
                           GroundSelectedPredicateAnnotations& delta_and_annot) const;

    void update_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> program_head,
                           ygg::ClosedInterval<ygg::float_t> interval,
                           const GroundAndAnnotationContext& context,
                           GroundSelectedFunctionAnnotations& delta_numeric_and_annot) const;
};

template<typename AggregationFunction>
class AchieverAndAnnotationPolicy<GroundTag, AggregationFunction> : public AndAnnotationPolicy<GroundTag, AggregationFunction>
{
public:
    using Atom = ::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>;
    using AtomIndex = ygg::Index<::tyr::formalism::datalog::GroundAtom<::tyr::formalism::FluentTag>>;
    using Achievers = std::vector<GroundWitnessAnnotation>;

    void clear_achievers() noexcept;

    const Achievers* find_achievers(Atom program_head) const noexcept;

    void record_achiever(Atom program_head, const GroundAndAnnotationContext& context) const;

private:
    mutable ygg::UnorderedMap<AtomIndex, Achievers> achievers;
};

static_assert(OrAnnotationPolicyConcept<NoOrAnnotationPolicy<GroundTag>, GroundTag>);
static_assert(AndAnnotationPolicyConcept<NoAndAnnotationPolicy<GroundTag>, GroundTag>);
static_assert(OrAnnotationPolicyConcept<OrAnnotationPolicy<GroundTag>, GroundTag>);
static_assert(AndAnnotationPolicyConcept<AndAnnotationPolicy<GroundTag, SumAggregation>, GroundTag>);
static_assert(AndAnnotationPolicyConcept<AchieverAndAnnotationPolicy<GroundTag, MaxAggregation>, GroundTag>);

}

#endif
