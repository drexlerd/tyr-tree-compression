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
#include "tyr/datalog/lifted/policies/aggregation.hpp"
#include "tyr/datalog/policies/annotation_concept.hpp"

#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>

namespace tyr::datalog
{

template<>
class NoOrAnnotationPolicy<GroundTag>
{
public:
    void initialize_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>, GroundSelectedPredicateAnnotations&) const noexcept {}

    void initialize_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>,
                               ygg::ClosedInterval<ygg::float_t>,
                               GroundSelectedFunctionAnnotations&) const noexcept
    {
    }

    GroundCostUpdate update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                                       const GroundAnnotation&,
                                       GroundSelectedPredicateAnnotations&) const noexcept
    {
        return GroundCostUpdate();
    }
};

template<>
class NoAndAnnotationPolicy<GroundTag>
{
public:
    void clear_achievers() noexcept {}

    void record_achiever(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>, const GroundAndAnnotationContext&) const noexcept {}

    void update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                           const GroundAndAnnotationContext&,
                           GroundSelectedPredicateAnnotations&) const noexcept
    {
    }

    void update_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>,
                           ygg::ClosedInterval<ygg::float_t>,
                           const GroundAndAnnotationContext&,
                           GroundSelectedFunctionAnnotations&) const noexcept
    {
    }
};

template<>
class OrAnnotationPolicy<GroundTag>
{
public:
    void initialize_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                               GroundSelectedPredicateAnnotations& program_and_annot) const
    {
        program_and_annot.insert_or_assign(program_head, GroundBaseAnnotation(Cost(0)));
    }

    void initialize_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> program_head,
                               ygg::ClosedInterval<ygg::float_t> interval,
                               GroundSelectedFunctionAnnotations& program_numeric_and_annot) const
    {
        program_numeric_and_annot.insert(program_head, interval, GroundBaseAnnotation(Cost(0)));
    }

    GroundCostUpdate update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                                       const GroundAnnotation& delta_and_annot,
                                       GroundSelectedPredicateAnnotations& program_and_annot) const
    {
        const auto new_cost = get_cost(delta_and_annot);
        if (const auto* old_annotation = program_and_annot.find(program_head))
        {
            const auto old_cost = get_cost(*old_annotation);
            if (new_cost < old_cost)
            {
                program_and_annot.insert_or_assign(program_head, delta_and_annot);
                return GroundCostUpdate(old_cost, new_cost);
            }
            return GroundCostUpdate(old_cost, old_cost);
        }

        program_and_annot.insert_or_assign(program_head, delta_and_annot);
        return GroundCostUpdate(std::nullopt, new_cost);
    }
};

template<typename AggregationFunction>
class AndAnnotationPolicy<GroundTag, AggregationFunction>
{
public:
    static constexpr AggregationFunction agg = AggregationFunction {};

    void clear_achievers() noexcept {}

    void record_achiever(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>, const GroundAndAnnotationContext&) const noexcept {}

    void update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                           const GroundAndAnnotationContext& context,
                           GroundSelectedPredicateAnnotations& delta_and_annot) const
    {
        delta_and_annot.insert_or_assign(program_head, GroundWitnessAnnotation(context.rule, context.metric, context.current_cost, context.numeric_supports));
    }

    void update_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> program_head,
                           ygg::ClosedInterval<ygg::float_t> interval,
                           const GroundAndAnnotationContext& context,
                           GroundSelectedFunctionAnnotations& delta_numeric_and_annot) const
    {
        delta_numeric_and_annot.insert(program_head, interval, GroundWitnessAnnotation(context.rule, context.metric, context.current_cost, context.numeric_supports));
    }
};

template<typename AggregationFunction>
class AchieverAndAnnotationPolicy<GroundTag, AggregationFunction> : public AndAnnotationPolicy<GroundTag, AggregationFunction>
{
public:
    using Atom = ::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>;
    using AtomIndex = ygg::Index<::tyr::formalism::datalog::GroundAtom<::tyr::formalism::FluentTag>>;
    using Achievers = std::vector<GroundWitnessAnnotation>;

    void clear_achievers() noexcept { achievers.clear(); }

    const Achievers* find_achievers(Atom program_head) const noexcept
    {
        const auto it = achievers.find(program_head.get_index());
        return it == achievers.end() ? nullptr : &it->second;
    }

    void record_achiever(Atom program_head, const GroundAndAnnotationContext& context) const
    {
        achievers[program_head.get_index()].emplace_back(context.rule, context.metric, context.current_cost, context.numeric_supports);
    }

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
