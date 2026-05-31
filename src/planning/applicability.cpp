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

#include "tyr/planning/applicability.hpp"

#include "tyr/planning/ground_task.hpp"
#include "tyr/planning/ground_task/state_builder.hpp"
#include "tyr/planning/lifted_task.hpp"
#include "tyr/planning/lifted_task/state_builder.hpp"

#ifndef TYR_HEADER_INSTANTIATION

namespace tyr::planning
{

template ygg::float_t evaluate(ygg::float_t element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(ygg::float_t element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundUnaryOperatorView<::tyr::formalism::Sub> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundUnaryOperatorView<::tyr::formalism::Sub> element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Add> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Sub> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Mul> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Div> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Add> element, const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Sub> element, const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Mul> element, const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Div> element, const StateContext<GroundTag>& context);

template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Eq> element, const StateContext<LiftedTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Ne> element, const StateContext<LiftedTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Ge> element, const StateContext<LiftedTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Gt> element, const StateContext<LiftedTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Le> element, const StateContext<LiftedTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Lt> element, const StateContext<LiftedTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Eq> element, const StateContext<GroundTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Ne> element, const StateContext<GroundTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Ge> element, const StateContext<GroundTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Gt> element, const StateContext<GroundTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Le> element, const StateContext<GroundTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBinaryOperatorView<::tyr::formalism::Lt> element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundMultiOperatorView<::tyr::formalism::Add> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundMultiOperatorView<::tyr::formalism::Mul> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundMultiOperatorView<::tyr::formalism::Add> element, const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundMultiOperatorView<::tyr::formalism::Mul> element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::StaticTag> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::StaticTag> element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::FluentTag> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::FluentTag> element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::AuxiliaryTag> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::AuxiliaryTag> element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundFunctionExpressionView element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundFunctionExpressionView element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundArithmeticOperatorView element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundArithmeticOperatorView element, const StateContext<GroundTag>& context);

template bool is_applicable(::tyr::formalism::planning::GroundBooleanOperatorView element, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundBooleanOperatorView element, const StateContext<GroundTag>& context);

template bool evaluate(::tyr::formalism::planning::GroundBooleanOperatorView element, const StateContext<LiftedTag>& context);
template bool evaluate(::tyr::formalism::planning::GroundBooleanOperatorView element, const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Assign, ::tyr::formalism::FluentTag> element,
                          const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::FluentTag> element,
                          const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Decrease, ::tyr::formalism::FluentTag> element,
                          const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::ScaleUp, ::tyr::formalism::FluentTag> element,
                          const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::ScaleDown, ::tyr::formalism::FluentTag> element,
                          const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::AuxiliaryTag> element,
                          const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Assign, ::tyr::formalism::FluentTag> element,
                          const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::FluentTag> element,
                          const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Decrease, ::tyr::formalism::FluentTag> element,
                          const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::ScaleUp, ::tyr::formalism::FluentTag> element,
                          const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::ScaleDown, ::tyr::formalism::FluentTag> element,
                          const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::AuxiliaryTag> element,
                          const StateContext<GroundTag>& context);

template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectOperatorView<::tyr::formalism::FluentTag> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectOperatorView<::tyr::formalism::AuxiliaryTag> element, const StateContext<LiftedTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectOperatorView<::tyr::formalism::FluentTag> element, const StateContext<GroundTag>& context);
template ygg::float_t evaluate(::tyr::formalism::planning::GroundNumericEffectOperatorView<::tyr::formalism::AuxiliaryTag> element, const StateContext<GroundTag>& context);

/**
 * is_applicable_if_fires
 */

template bool is_applicable_if_fires(::tyr::formalism::planning::GroundConditionalEffectView element,
                                     const StateContext<LiftedTag>& context,
                                     ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable_if_fires(::tyr::formalism::planning::GroundConditionalEffectView element,
                                     const StateContext<GroundTag>& context,
                                     ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);

template bool is_applicable_if_fires(::tyr::formalism::planning::GroundConditionalEffectListView elements,
                                     const StateContext<LiftedTag>& context,
                                     ::tyr::formalism::planning::EffectFamilyList& out_fluent_effect_families);
template bool is_applicable_if_fires(::tyr::formalism::planning::GroundConditionalEffectListView elements,
                                     const StateContext<GroundTag>& context,
                                     ::tyr::formalism::planning::EffectFamilyList& out_fluent_effect_families);

/**
 * is_applicable
 */

template bool is_applicable(::tyr::formalism::planning::GroundLiteralView<::tyr::formalism::StaticTag> element, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundLiteralView<::tyr::formalism::StaticTag> element, const StateContext<GroundTag>& context);

template bool is_applicable(::tyr::formalism::planning::GroundLiteralView<::tyr::formalism::DerivedTag> element, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundLiteralView<::tyr::formalism::DerivedTag> element, const StateContext<GroundTag>& context);

template bool is_applicable(::tyr::formalism::planning::GroundLiteralListView<::tyr::formalism::StaticTag> elements, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundLiteralListView<::tyr::formalism::DerivedTag> elements, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundLiteralListView<::tyr::formalism::StaticTag> elements, const StateContext<GroundTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundLiteralListView<::tyr::formalism::DerivedTag> elements, const StateContext<GroundTag>& context);

template bool is_applicable<::tyr::formalism::PositiveTag>(::tyr::formalism::planning::FDRFactView<::tyr::formalism::FluentTag> element, const StateContext<LiftedTag>& context);
template bool is_applicable<::tyr::formalism::NegativeTag>(::tyr::formalism::planning::FDRFactView<::tyr::formalism::FluentTag> element, const StateContext<LiftedTag>& context);
template bool is_applicable<::tyr::formalism::PositiveTag>(::tyr::formalism::planning::FDRFactView<::tyr::formalism::FluentTag> element, const StateContext<GroundTag>& context);
template bool is_applicable<::tyr::formalism::NegativeTag>(::tyr::formalism::planning::FDRFactView<::tyr::formalism::FluentTag> element, const StateContext<GroundTag>& context);

template bool is_applicable<::tyr::formalism::PositiveTag>(::tyr::formalism::planning::FDRFactListView<::tyr::formalism::FluentTag> elements,
                                                    const StateContext<LiftedTag>& context);
template bool is_applicable<::tyr::formalism::NegativeTag>(::tyr::formalism::planning::FDRFactListView<::tyr::formalism::FluentTag> elements,
                                                    const StateContext<LiftedTag>& context);
template bool is_applicable<::tyr::formalism::PositiveTag>(::tyr::formalism::planning::FDRFactListView<::tyr::formalism::FluentTag> elements,
                                                    const StateContext<GroundTag>& context);
template bool is_applicable<::tyr::formalism::NegativeTag>(::tyr::formalism::planning::FDRFactListView<::tyr::formalism::FluentTag> elements,
                                                    const StateContext<GroundTag>& context);

template bool is_applicable(::tyr::formalism::planning::GroundBooleanOperatorListView elements, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundBooleanOperatorListView elements, const StateContext<GroundTag>& context);

template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Assign, ::tyr::formalism::FluentTag> element,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::FluentTag> element,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Decrease, ::tyr::formalism::FluentTag> element,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::ScaleUp, ::tyr::formalism::FluentTag> element,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::ScaleDown, ::tyr::formalism::FluentTag> element,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Assign, ::tyr::formalism::FluentTag> element,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::FluentTag> element,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Decrease, ::tyr::formalism::FluentTag> element,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::ScaleUp, ::tyr::formalism::FluentTag> element,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::ScaleDown, ::tyr::formalism::FluentTag> element,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);

template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectOperatorView<::tyr::formalism::FluentTag> element,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectOperatorView<::tyr::formalism::FluentTag> element,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);

template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectOperatorListView<::tyr::formalism::FluentTag> elements,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectOperatorListView<::tyr::formalism::FluentTag> elements,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);

template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::AuxiliaryTag> element,
                            const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::AuxiliaryTag> element,
                            const StateContext<GroundTag>& context);

template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectOperatorView<::tyr::formalism::AuxiliaryTag> element, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundNumericEffectOperatorView<::tyr::formalism::AuxiliaryTag> element, const StateContext<GroundTag>& context);

// GroundConjunctiveCondition

template bool is_applicable(::tyr::formalism::planning::GroundConjunctiveConditionView element, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundConjunctiveConditionView element, const StateContext<GroundTag>& context);

// GroundConjunctiveEffect

template bool is_applicable(::tyr::formalism::planning::GroundConjunctiveEffectView element,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundConjunctiveEffectView element,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);

// GroundAction

template bool is_applicable(::tyr::formalism::planning::GroundActionView element,
                            const StateContext<LiftedTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& out_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::GroundActionView element,
                            const StateContext<GroundTag>& context,
                            ::tyr::formalism::planning::EffectFamilyList& out_fluent_effect_families);

// GroundAxiom

template bool is_applicable(::tyr::formalism::planning::GroundAxiomView element, const StateContext<LiftedTag>& context);
template bool is_applicable(::tyr::formalism::planning::GroundAxiomView element, const StateContext<GroundTag>& context);

/**
 * is_dynamically_applicable
 */

// GroundConjunctiveCondition

template bool is_dynamically_applicable(::tyr::formalism::planning::GroundConjunctiveConditionView element, const StateContext<LiftedTag>& context);
template bool is_dynamically_applicable(::tyr::formalism::planning::GroundConjunctiveConditionView element, const StateContext<GroundTag>& context);

}

#endif
