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

#include "tyr/datalog/applicability.hpp"

#ifndef TYR_HEADER_INSTANTIATION

namespace tyr::datalog
{

/**
 * evaluate
 */

template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundUnaryOperatorView<::tyr::formalism::Sub> element, const FactSets& fact_sets);

template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Add> element, const FactSets& fact_sets);
template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Sub> element, const FactSets& fact_sets);
template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Mul> element, const FactSets& fact_sets);
template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Div> element, const FactSets& fact_sets);

template bool evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Eq> element, const FactSets& fact_sets);
template bool evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Ne> element, const FactSets& fact_sets);
template bool evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Ge> element, const FactSets& fact_sets);
template bool evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Gt> element, const FactSets& fact_sets);
template bool evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Le> element, const FactSets& fact_sets);
template bool evaluate(::tyr::formalism::datalog::GroundBinaryOperatorView<::tyr::formalism::Lt> element, const FactSets& fact_sets);

template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundMultiOperatorView<::tyr::formalism::Add> element, const FactSets& fact_sets);
template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundMultiOperatorView<::tyr::formalism::Mul> element, const FactSets& fact_sets);

template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::StaticTag> element, const FactSets& fact_sets);
template ygg::ClosedInterval<ygg::float_t> evaluate(::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> element, const FactSets& fact_sets);

/**
 * is_applicable
 */

template bool is_applicable(::tyr::formalism::datalog::GroundLiteralView<::tyr::formalism::StaticTag> element, const FactSets& fact_sets);
template bool is_applicable(::tyr::formalism::datalog::GroundLiteralView<::tyr::formalism::FluentTag> element, const FactSets& fact_sets);

template bool is_applicable(::tyr::formalism::datalog::GroundLiteralListView<::tyr::formalism::StaticTag> elements, const FactSets& fact_sets);
template bool is_applicable(::tyr::formalism::datalog::GroundLiteralListView<::tyr::formalism::FluentTag> elements, const FactSets& fact_sets);

// GroundConjunctiveCondition

// GroundRule

/**
 * is_valid_binding
 */

template bool
is_valid_binding(::tyr::formalism::datalog::LiteralView<::tyr::formalism::StaticTag> element, const FactSets& fact_sets, ::tyr::formalism::datalog::GrounderContext& context);
template bool
is_valid_binding(::tyr::formalism::datalog::LiteralView<::tyr::formalism::FluentTag> element, const FactSets& fact_sets, ::tyr::formalism::datalog::GrounderContext& context);

template bool
is_valid_binding(::tyr::formalism::datalog::LiteralListView<::tyr::formalism::StaticTag> elements, const FactSets& fact_sets, ::tyr::formalism::datalog::GrounderContext& context);
template bool
is_valid_binding(::tyr::formalism::datalog::LiteralListView<::tyr::formalism::FluentTag> elements, const FactSets& fact_sets, ::tyr::formalism::datalog::GrounderContext& context);

template ygg::ClosedInterval<ygg::float_t> is_valid_binding(::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Assign, ::tyr::formalism::FluentTag> element,
                                                  const FactSets& fact_sets,
                                                  ::tyr::formalism::datalog::GrounderContext& context);
template ygg::ClosedInterval<ygg::float_t> is_valid_binding(::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::FluentTag> element,
                                                  const FactSets& fact_sets,
                                                  ::tyr::formalism::datalog::GrounderContext& context);
template ygg::ClosedInterval<ygg::float_t> is_valid_binding(::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Decrease, ::tyr::formalism::FluentTag> element,
                                                  const FactSets& fact_sets,
                                                  ::tyr::formalism::datalog::GrounderContext& context);
template ygg::ClosedInterval<ygg::float_t> is_valid_binding(::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::ScaleUp, ::tyr::formalism::FluentTag> element,
                                                  const FactSets& fact_sets,
                                                  ::tyr::formalism::datalog::GrounderContext& context);
template ygg::ClosedInterval<ygg::float_t> is_valid_binding(::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::ScaleDown, ::tyr::formalism::FluentTag> element,
                                                  const FactSets& fact_sets,
                                                  ::tyr::formalism::datalog::GrounderContext& context);
template ygg::ClosedInterval<ygg::float_t> is_valid_binding(::tyr::formalism::datalog::NumericEffectOperatorView<::tyr::formalism::FluentTag> element,
                                                  const FactSets& fact_sets,
                                                  ::tyr::formalism::datalog::GrounderContext& context);

}

#endif
