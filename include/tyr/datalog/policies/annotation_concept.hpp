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

#include <yggdrasil/core/config.hpp>
#include <yggdrasil/containers/vector.hpp>
#include "tyr/datalog/policies/aggregation.hpp"
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

// circle "or"-node
template<typename T>
concept OrAnnotationPolicyConcept = requires(const T& p,
                                             ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
                                             ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> program_function_head,
                                             ygg::ClosedInterval<ygg::float_t> interval,
                                             ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> delta_head,
                                             const SelectedPredicateAnnotations& delta_and_annot,
                                             SelectedPredicateAnnotations& program_and_annot,
                                             SelectedFunctionAnnotations& program_numeric_and_annot) {
    /// Annotate the initial cost of the atom.
    { p.initialize_annotation(program_head, program_and_annot) } -> std::same_as<void>;
    /// Annotate the initial cost of a numeric binding.
    { p.initialize_annotation(program_function_head, interval, program_numeric_and_annot) } -> std::same_as<void>;
    /// Annotate the cost of the atom from the given witness and annotations.
    /// `delta_head` indexes into the rule-local delta repository; `head` indexes into the global program repository.
    { p.update_annotation(program_head, delta_head, delta_and_annot, program_and_annot) } -> std::same_as<CostUpdate>;
};

// rectangular "and"-node
template<typename T>
concept AndAnnotationPolicyConcept = requires(const T& p,
                                              ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag> program_head,
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
                                              ::tyr::formalism::datalog::GrounderContext& iteration_context) {
    /// Ground the witness and annotate the cost of it from the given annotations.
    {
        p.update_annotation(program_head,
                            delta_head,
                            current_cost,
                            rule,
                            witness_condition,
                            numeric_support_selector,
                            numeric_support_selector_workspace,
                            program_and_annot,
                            program_numeric_and_annot,
                            delta_and_annot,
                            delta_context,
                            iteration_context)
    } -> std::same_as<void>;
};

}

#endif
