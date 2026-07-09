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

#include "tyr/datalog/ground/policies/annotation.hpp"

#include "tyr/datalog/policies/annotation.hpp"

namespace tyr::datalog
{

void NoOrAnnotationPolicy<GroundTag>::initialize_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                                                            GroundSelectedPredicateAnnotations&) const noexcept
{
}

void NoOrAnnotationPolicy<GroundTag>::initialize_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>,
                                                            ygg::ClosedInterval<ygg::float_t>,
                                                            GroundSelectedFunctionAnnotations&) const noexcept
{
}

GroundCostUpdate NoOrAnnotationPolicy<GroundTag>::update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                                                                    const GroundAnnotation&,
                                                                    GroundSelectedPredicateAnnotations&) const noexcept
{
    return GroundCostUpdate();
}

void NoAndAnnotationPolicy<GroundTag>::clear_achievers() noexcept {}

void NoAndAnnotationPolicy<GroundTag>::record_achiever(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                                                       const GroundAndAnnotationContext&) const noexcept
{
}

void NoAndAnnotationPolicy<GroundTag>::update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                                                         const GroundAndAnnotationContext&,
                                                         GroundSelectedPredicateAnnotations&) const noexcept
{
}

void NoAndAnnotationPolicy<GroundTag>::update_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>,
                                                         ygg::ClosedInterval<ygg::float_t>,
                                                         const GroundAndAnnotationContext&,
                                                         GroundSelectedFunctionAnnotations&) const noexcept
{
}

void OrAnnotationPolicy<GroundTag>::initialize_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                                                          GroundSelectedPredicateAnnotations& program_and_annot) const
{
    program_and_annot.insert_or_assign(program_head, GroundBaseAnnotation(Cost(0)));
}

void OrAnnotationPolicy<GroundTag>::initialize_annotation(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> program_head,
                                                          ygg::ClosedInterval<ygg::float_t> interval,
                                                          GroundSelectedFunctionAnnotations& program_numeric_and_annot) const
{
    program_numeric_and_annot.insert(program_head, interval, GroundBaseAnnotation(Cost(0)));
}

GroundCostUpdate OrAnnotationPolicy<GroundTag>::update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
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
        if (new_cost == old_cost)
            if (const auto* witness = std::get_if<GroundWitnessAnnotation>(&delta_and_annot); witness && witness_wins_tie<GroundTag>(*witness, old_annotation))
                program_and_annot.insert_or_assign(program_head, delta_and_annot);
        return GroundCostUpdate(old_cost, old_cost);
    }

    program_and_annot.insert_or_assign(program_head, delta_and_annot);
    return GroundCostUpdate(std::nullopt, new_cost);
}

template<typename AggregationFunction>
void AndAnnotationPolicy<GroundTag, AggregationFunction>::clear_achievers() noexcept
{
}

template<typename AggregationFunction>
void AndAnnotationPolicy<GroundTag, AggregationFunction>::record_achiever(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>,
                                                                          const GroundAndAnnotationContext&) const noexcept
{
}

template<typename AggregationFunction>
void AndAnnotationPolicy<GroundTag, AggregationFunction>::update_annotation(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> program_head,
                                                                            const GroundAndAnnotationContext& context,
                                                                            GroundSelectedPredicateAnnotations& delta_and_annot) const
{
    const auto best_global_cost = fetch_annotation_cost<GroundTag>(program_head, context.program_and_annot);
    const auto best_local_cost = fetch_annotation_cost<GroundTag>(program_head, delta_and_annot);
    const auto best_cost = std::min(best_global_cost, best_local_cost);
    if (best_cost < context.current_cost)
        return;

    const auto witness = GroundWitnessAnnotation(context.rule, context.metric, context.current_cost, context.numeric_supports);
    if (best_cost == context.current_cost
        && !witness_wins_tie<GroundTag>(
            witness,
            select_incumbent<GroundTag>(program_head, program_head, best_global_cost, best_local_cost, context.program_and_annot, delta_and_annot)))
        return;

    delta_and_annot.insert_or_assign(program_head, GroundAnnotation(witness));
}

template<typename AggregationFunction>
void AndAnnotationPolicy<GroundTag, AggregationFunction>::update_annotation(
    ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> program_head,
    ygg::ClosedInterval<ygg::float_t> interval,
    const GroundAndAnnotationContext& context,
    GroundSelectedFunctionAnnotations& delta_numeric_and_annot) const
{
    delta_numeric_and_annot.insert(program_head,
                                   interval,
                                   GroundWitnessAnnotation(context.rule, context.metric, context.current_cost, context.numeric_supports));
}

template<typename AggregationFunction>
void AchieverAndAnnotationPolicy<GroundTag, AggregationFunction>::clear_achievers() noexcept
{
    achievers.clear();
}

template<typename AggregationFunction>
const typename AchieverAndAnnotationPolicy<GroundTag, AggregationFunction>::Achievers*
AchieverAndAnnotationPolicy<GroundTag, AggregationFunction>::find_achievers(Atom program_head) const noexcept
{
    const auto it = achievers.find(program_head.get_index());
    return it == achievers.end() ? nullptr : &it->second;
}

template<typename AggregationFunction>
void AchieverAndAnnotationPolicy<GroundTag, AggregationFunction>::record_achiever(Atom program_head, const GroundAndAnnotationContext& context) const
{
    achievers[program_head.get_index()].emplace_back(context.rule, context.metric, context.current_cost, context.numeric_supports);
}

template class AndAnnotationPolicy<GroundTag, SumAggregation>;
template class AndAnnotationPolicy<GroundTag, MaxAggregation>;
template class AchieverAndAnnotationPolicy<GroundTag, MaxAggregation>;

}
