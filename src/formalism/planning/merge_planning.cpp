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

#include "tyr/formalism/planning/merge_planning.hpp"

#ifndef TYR_HEADER_INSTANTIATION

namespace tyr::formalism::planning
{

template std::pair<PredicateView<StaticTag>, bool> merge_d2p(::tyr::formalism::datalog::PredicateView<StaticTag> element, MergePlanningContext& context);
template std::pair<PredicateView<FluentTag>, bool> merge_d2p(::tyr::formalism::datalog::PredicateView<FluentTag> element, MergePlanningContext& context);
template std::pair<PredicateView<DerivedTag>, bool> merge_d2p(::tyr::formalism::datalog::PredicateView<FluentTag> element, MergePlanningContext& context);

template std::pair<AtomView<StaticTag>, bool>
merge_d2p(::tyr::formalism::datalog::AtomView<StaticTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<StaticTag>, PredicateView<StaticTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<AtomView<FluentTag>, bool>
merge_d2p(::tyr::formalism::datalog::AtomView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<FluentTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<AtomView<DerivedTag>, bool>
merge_d2p(::tyr::formalism::datalog::AtomView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<DerivedTag>>& predicate_mapping,
          MergePlanningContext& context);

template std::pair<PredicateBindingView<StaticTag>, bool>
merge_d2p(::tyr::formalism::datalog::PredicateBindingView<StaticTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<StaticTag>, PredicateView<StaticTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<PredicateBindingView<FluentTag>, bool>
merge_d2p(::tyr::formalism::datalog::PredicateBindingView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<FluentTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<PredicateBindingView<DerivedTag>, bool>
merge_d2p(::tyr::formalism::datalog::PredicateBindingView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<DerivedTag>>& predicate_mapping,
          MergePlanningContext& context);

template std::pair<GroundAtomView<StaticTag>, bool>
merge_atom_d2p<StaticTag, StaticTag>(::tyr::formalism::datalog::PredicateBindingView<StaticTag> element,
                                     const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<StaticTag>, PredicateView<StaticTag>>& predicate_mapping,
                                     MergePlanningContext& context);
template std::pair<GroundAtomView<FluentTag>, bool>
merge_atom_d2p<FluentTag, FluentTag>(::tyr::formalism::datalog::PredicateBindingView<FluentTag> element,
                                     const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<FluentTag>>& predicate_mapping,
                                     MergePlanningContext& context);
template std::pair<GroundAtomView<DerivedTag>, bool>
merge_atom_d2p<FluentTag, DerivedTag>(::tyr::formalism::datalog::PredicateBindingView<FluentTag> element,
                                      const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<DerivedTag>>& predicate_mapping,
                                      MergePlanningContext& context);

template std::pair<GroundAtomView<StaticTag>, bool>
merge_d2p(::tyr::formalism::datalog::GroundAtomView<StaticTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<StaticTag>, PredicateView<StaticTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<GroundAtomView<FluentTag>, bool>
merge_d2p(::tyr::formalism::datalog::GroundAtomView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<FluentTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<GroundAtomView<DerivedTag>, bool>
merge_d2p(::tyr::formalism::datalog::GroundAtomView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<DerivedTag>>& predicate_mapping,
          MergePlanningContext& context);

template std::pair<LiteralView<StaticTag>, bool>
merge_d2p(::tyr::formalism::datalog::LiteralView<StaticTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<StaticTag>, PredicateView<StaticTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<LiteralView<FluentTag>, bool>
merge_d2p(::tyr::formalism::datalog::LiteralView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<FluentTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<LiteralView<DerivedTag>, bool>
merge_d2p(::tyr::formalism::datalog::LiteralView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<DerivedTag>>& predicate_mapping,
          MergePlanningContext& context);

template std::pair<GroundLiteralView<StaticTag>, bool>
merge_d2p(::tyr::formalism::datalog::GroundLiteralView<StaticTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<StaticTag>, PredicateView<StaticTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<GroundLiteralView<FluentTag>, bool>
merge_d2p(::tyr::formalism::datalog::GroundLiteralView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<FluentTag>>& predicate_mapping,
          MergePlanningContext& context);
template std::pair<GroundLiteralView<DerivedTag>, bool>
merge_d2p(::tyr::formalism::datalog::GroundLiteralView<FluentTag> element,
          const ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<FluentTag>, PredicateView<DerivedTag>>& predicate_mapping,
          MergePlanningContext& context);

// Numeric

template std::pair<FunctionView<StaticTag>, bool> merge_d2p(::tyr::formalism::datalog::FunctionView<StaticTag> element, MergePlanningContext& context);
template std::pair<FunctionView<FluentTag>, bool> merge_d2p(::tyr::formalism::datalog::FunctionView<FluentTag> element, MergePlanningContext& context);

template std::pair<FunctionTermView<StaticTag>, bool> merge_d2p(::tyr::formalism::datalog::FunctionTermView<StaticTag> element, MergePlanningContext& context);
template std::pair<FunctionTermView<FluentTag>, bool> merge_d2p(::tyr::formalism::datalog::FunctionTermView<FluentTag> element, MergePlanningContext& context);

template std::pair<FunctionBindingView<StaticTag>, bool> merge_d2p(::tyr::formalism::datalog::FunctionBindingView<StaticTag> element, MergePlanningContext& context);
template std::pair<FunctionBindingView<FluentTag>, bool> merge_d2p(::tyr::formalism::datalog::FunctionBindingView<FluentTag> element, MergePlanningContext& context);

template std::pair<GroundFunctionTermView<StaticTag>, bool> merge_d2p(::tyr::formalism::datalog::GroundFunctionTermView<StaticTag> element,
                                                                      MergePlanningContext& context);
template std::pair<GroundFunctionTermView<FluentTag>, bool> merge_d2p(::tyr::formalism::datalog::GroundFunctionTermView<FluentTag> element,
                                                                      MergePlanningContext& context);

template std::pair<GroundFunctionTermValueView<StaticTag>, bool> merge_d2p(::tyr::formalism::datalog::GroundFunctionTermValueView<StaticTag> element,
                                                                           MergePlanningContext& context);
template std::pair<GroundFunctionTermValueView<FluentTag>, bool> merge_d2p(::tyr::formalism::datalog::GroundFunctionTermValueView<FluentTag> element,
                                                                           MergePlanningContext& context);

template std::pair<UnaryOperatorView<Sub, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::UnaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<UnaryOperatorView<Sub, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::UnaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);

template std::pair<BinaryOperatorView<Eq, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Eq, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Ne, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Ne, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Ge, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Ge, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Gt, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Gt, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Le, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Le, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Lt, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Lt, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Add, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Add, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Sub, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Mul, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Div, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Div, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Eq, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Eq, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Ne, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Ne, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Ge, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Ge, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Gt, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Gt, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Le, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Le, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Lt, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Lt, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Add, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Add, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Sub, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Mul, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<BinaryOperatorView<Div, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::BinaryOperatorView<Div, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);

template std::pair<MultiOperatorView<Add, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::MultiOperatorView<Add, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<MultiOperatorView<Mul, ygg::Data<FunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::MultiOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template std::pair<MultiOperatorView<Add, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::MultiOperatorView<Add, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);
template std::pair<MultiOperatorView<Mul, ygg::Data<GroundFunctionExpression>>, bool>
merge_d2p(::tyr::formalism::datalog::MultiOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);

template ygg::Data<ArithmeticOperator<ygg::Data<FunctionExpression>>>
merge_d2p(::tyr::formalism::datalog::ArithmeticOperatorView<ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template ygg::Data<ArithmeticOperator<ygg::Data<GroundFunctionExpression>>>
merge_d2p(::tyr::formalism::datalog::ArithmeticOperatorView<ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);

template ygg::Data<BooleanOperator<ygg::Data<FunctionExpression>>>
merge_d2p(::tyr::formalism::datalog::BooleanOperatorView<ygg::Data<::tyr::formalism::datalog::FunctionExpression>> element, MergePlanningContext& context);
template ygg::Data<BooleanOperator<ygg::Data<GroundFunctionExpression>>>
merge_d2p(::tyr::formalism::datalog::BooleanOperatorView<ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>> element, MergePlanningContext& context);

}

#endif
