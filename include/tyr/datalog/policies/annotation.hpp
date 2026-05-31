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

#include <yggdrasil/core/config.hpp>
#include <yggdrasil/containers/vector.hpp>
#include "tyr/datalog/policies/aggregation.hpp"
#include "tyr/datalog/policies/annotation_concept.hpp"
#include "tyr/datalog/policies/annotation_types.hpp"
#include "tyr/formalism/datalog/declarations.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <algorithm>
#include <cassert>
#include <concepts>
#include <limits>
#include <optional>
#include <tuple>
#include <vector>

namespace tyr::datalog
{

class NumericSupportSelector;
class NumericSupportSelectorWorkspace;

class NoOrAnnotationPolicy
{
public:
    void initialize_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> head, SelectedPredicateAnnotations& program_and_annot) const noexcept {}
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

class NoAndAnnotationPolicy
{
public:
    void update_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                           ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                           ygg::uint_t current_cost,
                           ::tyr::formalism::datalog::RuleView rule,
                           ::tyr::formalism::datalog::ConjunctiveConditionView witness_condition,
                           const NumericSupportSelector& numeric_support_selector,
                           NumericSupportSelectorWorkspace& numeric_support_selector_workspace,
                           const SelectedPredicateAnnotations& program_and_annot,
                           const SelectedFunctionAnnotations& program_numeric_and_annot,
                           SelectedPredicateAnnotations& delta_and_annot,
                           ::tyr::formalism::datalog::GrounderContext& delta_context,
                           ::tyr::formalism::datalog::GrounderContext& iteration_context) const noexcept
    {
    }

    void update_annotation(::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> program_head,
                           ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> delta_head,
                           ygg::ClosedInterval<ygg::float_t> interval,
                           ygg::uint_t current_cost,
                           ::tyr::formalism::datalog::RuleView rule,
                           ::tyr::formalism::datalog::ConjunctiveConditionView witness_condition,
                           const NumericSupportSelector& numeric_support_selector,
                           NumericSupportSelectorWorkspace& numeric_support_selector_workspace,
                           const SelectedPredicateAnnotations& program_and_annot,
                           const SelectedFunctionAnnotations& program_numeric_and_annot,
                           SelectedFunctionAnnotations& delta_numeric_and_annot,
                           ::tyr::formalism::datalog::GrounderContext& delta_context,
                           ::tyr::formalism::datalog::GrounderContext& iteration_context) const
    {
    }
};

class OrAnnotationPolicy
{
public:
    void initialize_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head, SelectedPredicateAnnotations& program_and_annot) const;
    void initialize_annotation(::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> program_head,
                               ygg::ClosedInterval<ygg::float_t> interval,
                               SelectedFunctionAnnotations& program_numeric_and_annot) const;

    CostUpdate update_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                                 ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                                 const SelectedPredicateAnnotations& delta_and_annot,
                                 SelectedPredicateAnnotations& program_and_annot) const;
};

template<typename AggregationFunction>
class AndAnnotationPolicy
{
public:
    static constexpr AggregationFunction agg = AggregationFunction {};

    void update_annotation(::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                           ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                           ygg::uint_t current_cost,
                           ::tyr::formalism::datalog::RuleView rule,
                           ::tyr::formalism::datalog::ConjunctiveConditionView witness_condition,
                           const NumericSupportSelector& numeric_support_selector,
                           NumericSupportSelectorWorkspace& numeric_support_selector_workspace,
                           const SelectedPredicateAnnotations& program_and_annot,
                           const SelectedFunctionAnnotations& program_numeric_and_annot,
                           SelectedPredicateAnnotations& delta_and_annot,
                           ::tyr::formalism::datalog::GrounderContext& delta_context,
                           ::tyr::formalism::datalog::GrounderContext& iteration_context) const;

    void update_annotation(::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> program_head,
                           ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> delta_head,
                           ygg::ClosedInterval<ygg::float_t> interval,
                           ygg::uint_t current_cost,
                           ::tyr::formalism::datalog::RuleView rule,
                           ::tyr::formalism::datalog::ConjunctiveConditionView witness_condition,
                           const NumericSupportSelector& numeric_support_selector,
                           NumericSupportSelectorWorkspace& numeric_support_selector_workspace,
                           const SelectedPredicateAnnotations& program_and_annot,
                           const SelectedFunctionAnnotations& program_numeric_and_annot,
                           SelectedFunctionAnnotations& delta_numeric_and_annot,
                           ::tyr::formalism::datalog::GrounderContext& delta_context,
                           ::tyr::formalism::datalog::GrounderContext& iteration_context) const;
};

}

#endif
