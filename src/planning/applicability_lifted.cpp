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

#include "tyr/planning/applicability_lifted.hpp"

#include "tyr/planning/lifted/state_builder.hpp"
#include "tyr/planning/lifted_task.hpp"

#ifndef TYR_HEADER_INSTANTIATION

namespace tyr::planning
{

/**
 * evaluate
 */

// LiftedUnaryOperatorView

template ygg::float_t evaluate(::tyr::formalism::planning::LiftedUnaryOperatorView<::tyr::formalism::Sub> element, const ApplicabilityContext& context);

// LiftedBinaryOperatorView arithmetic

template ygg::float_t evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Add> element, const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Sub> element, const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Mul> element, const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Div> element, const ApplicabilityContext& context);

// LiftedBinaryOperatorView boolean

template bool evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Eq> element, const ApplicabilityContext& context);
template bool evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Ne> element, const ApplicabilityContext& context);
template bool evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Ge> element, const ApplicabilityContext& context);
template bool evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Gt> element, const ApplicabilityContext& context);
template bool evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Le> element, const ApplicabilityContext& context);
template bool evaluate(::tyr::formalism::planning::LiftedBinaryOperatorView<::tyr::formalism::Lt> element, const ApplicabilityContext& context);

// LiftedMultiOperatorView

template ygg::float_t evaluate(::tyr::formalism::planning::LiftedMultiOperatorView<::tyr::formalism::Add> element, const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::LiftedMultiOperatorView<::tyr::formalism::Mul> element, const ApplicabilityContext& context);

// NumericEffectView

template ygg::float_t evaluate(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::Assign, ::tyr::formalism::FluentTag> element,
                               const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::FluentTag> element,
                               const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::Decrease, ::tyr::formalism::FluentTag> element,
                               const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::ScaleUp, ::tyr::formalism::FluentTag> element,
                               const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::ScaleDown, ::tyr::formalism::FluentTag> element,
                               const ApplicabilityContext& context);

template ygg::float_t evaluate(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::AuxiliaryTag> element,
                               const ApplicabilityContext& context);

// NumericEffectOperatorView

template ygg::float_t evaluate(::tyr::formalism::planning::NumericEffectOperatorView<::tyr::formalism::FluentTag> element, const ApplicabilityContext& context);
template ygg::float_t evaluate(::tyr::formalism::planning::NumericEffectOperatorView<::tyr::formalism::AuxiliaryTag> element,
                               const ApplicabilityContext& context);

/**
 * is_applicable
 */

// LiteralListView

template bool is_applicable(::tyr::formalism::planning::LiteralListView<::tyr::formalism::StaticTag> elements, const ApplicabilityContext& context);
template bool is_applicable(::tyr::formalism::planning::LiteralListView<::tyr::formalism::FluentTag> elements, const ApplicabilityContext& context);
template bool is_applicable(::tyr::formalism::planning::LiteralListView<::tyr::formalism::DerivedTag> elements, const ApplicabilityContext& context);

// NumericEffectView over fluent function terms

template bool is_applicable(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::Assign, ::tyr::formalism::FluentTag> element,
                            const ApplicabilityContext& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::Increase, ::tyr::formalism::FluentTag> element,
                            const ApplicabilityContext& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::Decrease, ::tyr::formalism::FluentTag> element,
                            const ApplicabilityContext& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::ScaleUp, ::tyr::formalism::FluentTag> element,
                            const ApplicabilityContext& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);
template bool is_applicable(::tyr::formalism::planning::NumericEffectView<::tyr::formalism::ScaleDown, ::tyr::formalism::FluentTag> element,
                            const ApplicabilityContext& context,
                            ::tyr::formalism::planning::EffectFamilyList& ref_fluent_effect_families);

}

#endif
