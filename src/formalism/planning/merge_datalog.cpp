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

#include "tyr/formalism/planning/merge_datalog.hpp"

#ifndef TYR_HEADER_INSTANTIATION

namespace tyr::formalism::planning
{

template std::pair<::tyr::formalism::datalog::PredicateView<StaticTag>, bool> merge_p2d(PredicateView<StaticTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::PredicateView<FluentTag>, bool> merge_p2d(PredicateView<FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::PredicateView<FluentTag>, bool> merge_p2d(PredicateView<DerivedTag> element, MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::AtomView<StaticTag>, bool>
merge_p2d(AtomView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::AtomView<FluentTag>, bool>
merge_p2d(AtomView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::AtomView<FluentTag>, bool>
merge_p2d(AtomView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::PredicateBindingView<StaticTag>, bool>
merge_p2d(PredicateBindingView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::PredicateBindingView<FluentTag>, bool>
merge_p2d(PredicateBindingView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::PredicateBindingView<FluentTag>, bool>
merge_p2d(PredicateBindingView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::GroundAtomView<StaticTag>, bool>
merge_p2d(GroundAtomView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundAtomView<FluentTag>, bool>
merge_p2d(GroundAtomView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundAtomView<FluentTag>, bool>
merge_p2d(GroundAtomView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::LiteralView<StaticTag>, bool>
merge_p2d(LiteralView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::LiteralView<FluentTag>, bool>
merge_p2d(LiteralView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::LiteralView<FluentTag>, bool>
merge_p2d(LiteralView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundLiteralView<StaticTag>, bool>
merge_p2d(GroundLiteralView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundLiteralView<FluentTag>, bool>
merge_p2d(GroundLiteralView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundLiteralView<FluentTag>, bool>
merge_p2d(GroundLiteralView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

std::optional<::tyr::formalism::datalog::GroundLiteralView<FluentTag>>
merge_p2d(FDRFactView<FluentTag> element,
          bool polarity,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& derived_predicate_mapping,
          MergeDatalogContext& context);

// Numeric

template std::pair<::tyr::formalism::datalog::FunctionView<StaticTag>, bool> merge_p2d(FunctionView<StaticTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::FunctionView<FluentTag>, bool> merge_p2d(FunctionView<FluentTag> element, MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::FunctionTermView<StaticTag>, bool> merge_p2d(FunctionTermView<StaticTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::FunctionTermView<FluentTag>, bool> merge_p2d(FunctionTermView<FluentTag> element, MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::FunctionBindingView<StaticTag>, bool> merge_p2d(FunctionBindingView<StaticTag> element,
                                                                                              MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::FunctionBindingView<FluentTag>, bool> merge_p2d(FunctionBindingView<FluentTag> element,
                                                                                              MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::GroundFunctionTermView<StaticTag>, bool> merge_p2d(GroundFunctionTermView<StaticTag> element,
                                                                                                 MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundFunctionTermView<FluentTag>, bool> merge_p2d(GroundFunctionTermView<FluentTag> element,
                                                                                                 MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::GroundFunctionTermValueView<StaticTag>, bool> merge_p2d(GroundFunctionTermValueView<StaticTag> element,
                                                                                                      MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundFunctionTermValueView<FluentTag>, bool> merge_p2d(GroundFunctionTermValueView<FluentTag> element,
                                                                                                      MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::FunctionView<FluentTag>, bool> merge_p2d<AuxiliaryTag, FluentTag>(FunctionView<AuxiliaryTag> element,
                                                                                                                MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::FunctionTermView<FluentTag>, bool> merge_p2d<AuxiliaryTag, FluentTag>(FunctionTermView<AuxiliaryTag> element,
                                                                                                                    MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::FunctionBindingView<FluentTag>, bool>
merge_p2d<AuxiliaryTag, FluentTag>(FunctionBindingView<AuxiliaryTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundFunctionTermView<FluentTag>, bool>
merge_p2d<AuxiliaryTag, FluentTag>(GroundFunctionTermView<AuxiliaryTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundFunctionTermValueView<FluentTag>, bool>
merge_p2d<AuxiliaryTag, FluentTag>(GroundFunctionTermValueView<AuxiliaryTag> element, MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::NumericEffectView<Assign, FluentTag>, bool>
merge_p2d<Assign, FluentTag, FluentTag>(NumericEffectView<Assign, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::NumericEffectView<Increase, FluentTag>, bool>
merge_p2d<Increase, FluentTag, FluentTag>(NumericEffectView<Increase, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::NumericEffectView<Decrease, FluentTag>, bool>
merge_p2d<Decrease, FluentTag, FluentTag>(NumericEffectView<Decrease, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::NumericEffectView<ScaleUp, FluentTag>, bool>
merge_p2d<ScaleUp, FluentTag, FluentTag>(NumericEffectView<ScaleUp, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::NumericEffectView<ScaleDown, FluentTag>, bool>
merge_p2d<ScaleDown, FluentTag, FluentTag>(NumericEffectView<ScaleDown, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::NumericEffectView<Increase, FluentTag>, bool>
merge_p2d<Increase, AuxiliaryTag, FluentTag>(NumericEffectView<Increase, AuxiliaryTag> element, MergeDatalogContext& context);
template ygg::Data<::tyr::formalism::datalog::NumericEffectOperator<FluentTag>> merge_p2d<FluentTag, FluentTag>(NumericEffectOperatorView<FluentTag> element,
                                                                                                                MergeDatalogContext& context);
template ygg::Data<::tyr::formalism::datalog::NumericEffectOperator<FluentTag>>
merge_p2d<AuxiliaryTag, FluentTag>(NumericEffectOperatorView<AuxiliaryTag> element, MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::GroundNumericEffectView<Assign, FluentTag>, bool>
merge_p2d<Assign, FluentTag, FluentTag>(GroundNumericEffectView<Assign, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundNumericEffectView<Increase, FluentTag>, bool>
merge_p2d<Increase, FluentTag, FluentTag>(GroundNumericEffectView<Increase, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundNumericEffectView<Decrease, FluentTag>, bool>
merge_p2d<Decrease, FluentTag, FluentTag>(GroundNumericEffectView<Decrease, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundNumericEffectView<ScaleUp, FluentTag>, bool>
merge_p2d<ScaleUp, FluentTag, FluentTag>(GroundNumericEffectView<ScaleUp, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundNumericEffectView<ScaleDown, FluentTag>, bool>
merge_p2d<ScaleDown, FluentTag, FluentTag>(GroundNumericEffectView<ScaleDown, FluentTag> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::GroundNumericEffectView<Increase, FluentTag>, bool>
merge_p2d<Increase, AuxiliaryTag, FluentTag>(GroundNumericEffectView<Increase, AuxiliaryTag> element, MergeDatalogContext& context);
template ygg::Data<::tyr::formalism::datalog::GroundNumericEffectOperator<FluentTag>>
merge_p2d<FluentTag, FluentTag>(GroundNumericEffectOperatorView<FluentTag> element, MergeDatalogContext& context);
template ygg::Data<::tyr::formalism::datalog::GroundNumericEffectOperator<FluentTag>>
merge_p2d<AuxiliaryTag, FluentTag>(GroundNumericEffectOperatorView<AuxiliaryTag> element, MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::UnaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(UnaryOperatorView<Sub, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::UnaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(UnaryOperatorView<Sub, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Eq, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Eq, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Ne, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Ne, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Ge, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Ge, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Gt, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Gt, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Le, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Le, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Lt, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Lt, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Add, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Add, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Sub, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Mul, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Div, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Div, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Eq, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Eq, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Ne, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Ne, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Ge, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Ge, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Gt, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Gt, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Le, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Le, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Lt, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Lt, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Add, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Add, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Sub, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Mul, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Div, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Div, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

template std::pair<::tyr::formalism::datalog::MultiOperatorView<Add, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(MultiOperatorView<Add, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::MultiOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(MultiOperatorView<Mul, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::MultiOperatorView<Add, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(MultiOperatorView<Add, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
template std::pair<::tyr::formalism::datalog::MultiOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(MultiOperatorView<Mul, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

template ygg::Data<::tyr::formalism::datalog::ArithmeticOperator<ygg::Data<::tyr::formalism::datalog::FunctionExpression>>>
merge_p2d(ArithmeticOperatorView<ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template ygg::Data<::tyr::formalism::datalog::ArithmeticOperator<ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>>
merge_p2d(ArithmeticOperatorView<ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

template ygg::Data<::tyr::formalism::datalog::BooleanOperator<ygg::Data<::tyr::formalism::datalog::FunctionExpression>>>
merge_p2d(BooleanOperatorView<ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
template ygg::Data<::tyr::formalism::datalog::BooleanOperator<ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>>
merge_p2d(BooleanOperatorView<ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

}

#endif
