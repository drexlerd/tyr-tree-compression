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

#ifndef TYR_FORMALISM_PLANNING_MERGE_DATALOG_HPP_
#define TYR_FORMALISM_PLANNING_MERGE_DATALOG_HPP_

#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/concepts.hpp>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>
#include <yggdrasil/containers/tuple.hpp>
#include "tyr/formalism/datalog/builder.hpp"
#include "tyr/formalism/datalog/canonicalization.hpp"
#include "tyr/formalism/datalog/declarations.hpp"
#include "tyr/formalism/datalog/repository.hpp"
#include "tyr/formalism/planning/declarations.hpp"
#include "tyr/formalism/planning/merge_datalog_decl.hpp"
#include "tyr/formalism/planning/repository.hpp"

#include <optional>

namespace tyr::formalism::planning
{

// Common

std::pair<::tyr::formalism::datalog::VariableView, bool> merge_p2d(VariableView element, MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::ObjectView, bool> merge_p2d(ObjectView element, MergeDatalogContext& context);

ygg::Data<Term> merge_p2d(TermView element, MergeDatalogContext& context);

// Propositional

template<FactKind T_SRC, FactKind T_DST = T_SRC>
std::pair<::tyr::formalism::datalog::PredicateView<T_DST>, bool> merge_p2d(PredicateView<T_SRC> element, MergeDatalogContext& context);

template<FactKind T_SRC, FactKind T_DST = T_SRC>
std::pair<::tyr::formalism::datalog::AtomView<T_DST>, bool>
merge_p2d(AtomView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context);

template<FactKind T_SRC, FactKind T_DST = T_SRC>
std::pair<::tyr::formalism::datalog::PredicateBindingView<T_DST>, bool>
merge_p2d(PredicateBindingView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context);

template<FactKind T_SRC, FactKind T_DST = T_SRC>
std::pair<::tyr::formalism::datalog::GroundAtomView<T_DST>, bool>
merge_p2d(GroundAtomView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context);

template<FactKind T_SRC, FactKind T_DST = T_SRC>
std::pair<::tyr::formalism::datalog::GroundAtomView<T_DST>, bool>
merge_p2d(GroundAtomView<T_SRC> element,  //
          ygg::UnorderedMap<GroundAtomView<T_SRC>, ::tyr::formalism::datalog::GroundAtomView<T_DST>>& atom_mapping,
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context);

template<FactKind T_SRC, FactKind T_DST = T_SRC>
std::pair<::tyr::formalism::datalog::LiteralView<T_DST>, bool>
merge_p2d(LiteralView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context);

template<FactKind T_SRC, FactKind T_DST = T_SRC>
std::pair<::tyr::formalism::datalog::GroundLiteralView<T_DST>, bool>
merge_p2d(GroundLiteralView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context);

template<FactKind T_SRC, FactKind T_DST = T_SRC>
std::pair<::tyr::formalism::datalog::GroundLiteralView<T_DST>, bool>
merge_p2d(GroundLiteralView<T_SRC> element,  //
          ygg::UnorderedMap<GroundAtomView<T_SRC>, ::tyr::formalism::datalog::GroundAtomView<T_DST>>& atom_mapping,
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context);

std::optional<::tyr::formalism::datalog::GroundLiteralView<FluentTag>>
merge_p2d(FDRFactView<FluentTag> element,
          bool polarity,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

std::optional<::tyr::formalism::datalog::GroundLiteralView<FluentTag>>
merge_p2d(FDRFactView<FluentTag> element,
          bool polarity,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& derived_predicate_mapping,
          MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& fluent_atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          ygg::UnorderedMap<GroundAtomView<DerivedTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& derived_atom_mapping,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& derived_predicate_mapping,
          MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& fluent_atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          MergeDatalogContext& context);

// Numeric

template<FactKind T>
std::pair<::tyr::formalism::datalog::FunctionView<T>, bool> merge_p2d(FunctionView<T> element, MergeDatalogContext& context);

template<FactKind T>
std::pair<::tyr::formalism::datalog::FunctionTermView<T>, bool> merge_p2d(FunctionTermView<T> element, MergeDatalogContext& context);

template<FactKind T>
std::pair<::tyr::formalism::datalog::FunctionBindingView<T>, bool> merge_p2d(FunctionBindingView<T> element, MergeDatalogContext& context);

template<FactKind T>
std::pair<::tyr::formalism::datalog::GroundFunctionTermView<T>, bool> merge_p2d(GroundFunctionTermView<T> element, MergeDatalogContext& context);

template<FactKind T>
std::pair<::tyr::formalism::datalog::GroundFunctionTermValueView<T>, bool> merge_p2d(GroundFunctionTermValueView<T> element, MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Assign, FluentTag>, bool> merge_p2d(NumericEffectView<Assign, FluentTag> element,
                                                                                               MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Increase, FluentTag>, bool> merge_p2d(NumericEffectView<Increase, FluentTag> element,
                                                                                                 MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Decrease, FluentTag>, bool> merge_p2d(NumericEffectView<Decrease, FluentTag> element,
                                                                                                 MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::ScaleUp, FluentTag>, bool> merge_p2d(NumericEffectView<ScaleUp, FluentTag> element,
                                                                                                MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::ScaleDown, FluentTag>, bool> merge_p2d(NumericEffectView<ScaleDown, FluentTag> element,
                                                                                                  MergeDatalogContext& context);

ygg::Data<::tyr::formalism::datalog::NumericEffectOperator<FluentTag>> merge_p2d(NumericEffectOperatorView<FluentTag> element, MergeDatalogContext& context);

ygg::Data<::tyr::formalism::datalog::FunctionExpression> merge_p2d(FunctionExpressionView element, MergeDatalogContext& context);

ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression> merge_p2d(GroundFunctionExpressionView element, MergeDatalogContext& context);

template<OpKind O, typename T>
std::pair<::tyr::formalism::datalog::UnaryOperatorView<O, to_datalog_payload_t<T>>, bool> merge_p2d(UnaryOperatorView<O, T> element, MergeDatalogContext& context);

template<OpKind O, typename T>
std::pair<::tyr::formalism::datalog::BinaryOperatorView<O, to_datalog_payload_t<T>>, bool> merge_p2d(BinaryOperatorView<O, T> element, MergeDatalogContext& context);

template<OpKind O, typename T>
std::pair<::tyr::formalism::datalog::MultiOperatorView<O, to_datalog_payload_t<T>>, bool> merge_p2d(MultiOperatorView<O, T> element, MergeDatalogContext& context);

template<typename T>
ygg::Data<::tyr::formalism::datalog::ArithmeticOperator<to_datalog_payload_t<T>>> merge_p2d(ArithmeticOperatorView<T> element, MergeDatalogContext& context);

template<typename T>
ygg::Data<::tyr::formalism::datalog::BooleanOperator<to_datalog_payload_t<T>>> merge_p2d(BooleanOperatorView<T> element, MergeDatalogContext& context);

// Common

inline std::pair<::tyr::formalism::datalog::VariableView, bool> merge_p2d(VariableView element, MergeDatalogContext& context)
{
    auto variable_ptr = context.builder.template get_builder<Variable>();
    auto& variable = *variable_ptr;
    variable.clear();

    variable.name = element.get_name();

    canonicalize(variable);
    return context.destination.get_or_create(variable);
}

inline std::pair<::tyr::formalism::datalog::ObjectView, bool> merge_p2d(ObjectView element, MergeDatalogContext& context)
{
    auto object_ptr = context.builder.template get_builder<Object>();
    auto& object = *object_ptr;
    object.clear();

    object.name = element.get_name();

    canonicalize(object);
    return context.destination.get_or_create(object);
}

inline ygg::Data<Term> merge_p2d(TermView element, MergeDatalogContext& context)
{
    return visit(
        [&](auto&& arg)
        {
            using Alternative = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<Alternative, ParameterIndex>)
                return ygg::Data<Term>(arg);
            else if constexpr (std::is_same_v<Alternative, ObjectView>)
                return ygg::Data<Term>(merge_p2d(arg, context).first.get_index());
            else
                static_assert(ygg::dependent_false<Alternative>::value, "Missing case");
        },
        element.get_variant());
}

// Propositional

template<FactKind T_SRC, FactKind T_DST>
std::pair<::tyr::formalism::datalog::PredicateView<T_DST>, bool> merge_p2d(PredicateView<T_SRC> element, MergeDatalogContext& context)
{
    auto predicate_ptr = context.builder.template get_builder<Predicate<T_DST>>();
    auto& predicate = *predicate_ptr;
    predicate.clear();

    predicate.name = element.get_name();
    predicate.arity = element.get_arity();

    canonicalize(predicate);
    return context.destination.get_or_create(predicate);
}

template<FactKind T_SRC, FactKind T_DST>
std::pair<::tyr::formalism::datalog::AtomView<T_DST>, bool>
merge_p2d(AtomView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context)
{
    auto atom_ptr = context.builder.template get_builder<::tyr::formalism::datalog::Atom<T_DST>>();
    auto& atom = *atom_ptr;
    atom.clear();

    atom.predicate = predicate_mapping.at(element.get_predicate()).get_index();
    for (const auto term : element.get_terms())
        atom.terms.push_back(merge_p2d(term, context));

    canonicalize(atom);
    return context.destination.get_or_create(atom);
}

template<FactKind T_SRC, FactKind T_DST>
std::pair<::tyr::formalism::datalog::PredicateBindingView<T_DST>, bool>
merge_p2d(PredicateBindingView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context)
{
    auto binding_ptr = context.builder.template get_builder<RelationBinding<Predicate<T_DST>>>();
    auto& binding = *binding_ptr;
    binding.clear();

    binding.relation = predicate_mapping.at(element.get_relation()).get_index();
    for (const auto object : element.get_objects())
        binding.objects.push_back(object.get_index());

    canonicalize(binding);
    return context.destination.get_or_create(binding);
}

template<FactKind T_SRC, FactKind T_DST>
std::pair<::tyr::formalism::datalog::GroundAtomView<T_DST>, bool>
merge_p2d(GroundAtomView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context)
{
    auto atom_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundAtom<T_DST>>();
    auto& atom = *atom_ptr;
    atom.clear();

    atom.binding = merge_p2d<T_SRC, T_DST>(element.get_row(), predicate_mapping, context).first.get_index();

    canonicalize(atom);
    return context.destination.get_or_create(atom);
}

template<FactKind T_SRC, FactKind T_DST>
std::pair<::tyr::formalism::datalog::GroundAtomView<T_DST>, bool>
merge_p2d(GroundAtomView<T_SRC> element,  //
          ygg::UnorderedMap<GroundAtomView<T_SRC>, ::tyr::formalism::datalog::GroundAtomView<T_DST>>& atom_mapping,
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context)
{
    if (const auto it = atom_mapping.find(element); it != atom_mapping.end())
        return std::make_pair(it->second, false);

    auto [atom, inserted] = merge_p2d<T_SRC, T_DST>(element, predicate_mapping, context);
    atom_mapping.emplace(element, atom);
    return std::make_pair(atom, inserted);
}

template<FactKind T_SRC, FactKind T_DST>
std::pair<::tyr::formalism::datalog::LiteralView<T_DST>, bool>
merge_p2d(LiteralView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context)
{
    auto literal_ptr = context.builder.template get_builder<::tyr::formalism::datalog::Literal<T_DST>>();
    auto& literal = *literal_ptr;
    literal.clear();

    literal.polarity = element.get_polarity();
    literal.atom = merge_p2d<T_SRC, T_DST>(element.get_atom(), predicate_mapping, context).first.get_index();

    canonicalize(literal);
    return context.destination.get_or_create(literal);
}

template<FactKind T_SRC, FactKind T_DST>
std::pair<::tyr::formalism::datalog::GroundLiteralView<T_DST>, bool>
merge_p2d(GroundLiteralView<T_SRC> element,  //
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context)
{
    auto literal_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundLiteral<T_DST>>();
    auto& literal = *literal_ptr;
    literal.clear();

    literal.polarity = element.get_polarity();
    literal.atom = merge_p2d<T_SRC, T_DST>(element.get_atom(), predicate_mapping, context).first.get_index();

    canonicalize(literal);
    return context.destination.get_or_create(literal);
}

template<FactKind T_SRC, FactKind T_DST>
std::pair<::tyr::formalism::datalog::GroundLiteralView<T_DST>, bool>
merge_p2d(GroundLiteralView<T_SRC> element,  //
          ygg::UnorderedMap<GroundAtomView<T_SRC>, ::tyr::formalism::datalog::GroundAtomView<T_DST>>& atom_mapping,
          const ygg::UnorderedMap<PredicateView<T_SRC>, ::tyr::formalism::datalog::PredicateView<T_DST>>& predicate_mapping,
          MergeDatalogContext& context)
{
    auto literal_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundLiteral<T_DST>>();
    auto& literal = *literal_ptr;
    literal.clear();

    literal.polarity = element.get_polarity();
    literal.atom = merge_p2d<T_SRC, T_DST>(element.get_atom(), atom_mapping, predicate_mapping, context).first.get_index();

    canonicalize(literal);
    return context.destination.get_or_create(literal);
}

inline std::optional<::tyr::formalism::datalog::GroundLiteralView<FluentTag>>
merge_p2d(FDRFactView<FluentTag> element,
          bool polarity,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context)
{
    if (!element.has_value())
        return std::nullopt;

    auto literal_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundLiteral<FluentTag>>();
    auto& literal = *literal_ptr;
    literal.clear();
    literal.polarity = polarity;
    literal.atom = merge_p2d(element.get_atom().value(), predicate_mapping, context).first.get_index();

    ::tyr::formalism::datalog::canonicalize(literal);
    return context.destination.get_or_create(literal).first;
}

inline std::optional<::tyr::formalism::datalog::GroundLiteralView<FluentTag>>
merge_p2d(FDRFactView<FluentTag> element,
          bool polarity,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context)
{
    if (!element.has_value())
        return std::nullopt;

    auto literal_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundLiteral<FluentTag>>();
    auto& literal = *literal_ptr;
    literal.clear();
    literal.polarity = polarity;
    literal.atom = merge_p2d(element.get_atom().value(), atom_mapping, predicate_mapping, context).first.get_index();

    ::tyr::formalism::datalog::canonicalize(literal);
    return context.destination.get_or_create(literal).first;
}

inline std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& derived_predicate_mapping,
          MergeDatalogContext& context)
{
    auto condition_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundConjunctiveCondition>();
    auto& condition = *condition_ptr;
    condition.clear();

    for (const auto fact : element.template get_facts<PositiveTag>())
        if (const auto literal = merge_p2d(fact, true, fluent_predicate_mapping, context))
            condition.fluent_literals.push_back(literal->get_index());

    for (const auto fact : element.template get_facts<NegativeTag>())
        if (const auto literal = merge_p2d(fact, false, fluent_predicate_mapping, context))
            condition.fluent_literals.push_back(literal->get_index());

    for (const auto literal : element.template get_literals<DerivedTag>())
        condition.fluent_literals.push_back(merge_p2d(literal, derived_predicate_mapping, context).first.get_index());

    for (const auto numeric_constraint : element.get_numeric_constraints())
        condition.numeric_constraints.push_back(merge_p2d(numeric_constraint, context));

    ::tyr::formalism::datalog::canonicalize(condition);
    return context.destination.get_or_create(condition);
}

inline std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& fluent_atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          ygg::UnorderedMap<GroundAtomView<DerivedTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& derived_atom_mapping,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& derived_predicate_mapping,
          MergeDatalogContext& context)
{
    auto condition_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundConjunctiveCondition>();
    auto& condition = *condition_ptr;
    condition.clear();

    for (const auto fact : element.template get_facts<PositiveTag>())
        if (const auto literal = merge_p2d(fact, true, fluent_atom_mapping, fluent_predicate_mapping, context))
            condition.fluent_literals.push_back(literal->get_index());

    for (const auto fact : element.template get_facts<NegativeTag>())
        if (const auto literal = merge_p2d(fact, false, fluent_atom_mapping, fluent_predicate_mapping, context))
            condition.fluent_literals.push_back(literal->get_index());

    for (const auto literal : element.template get_literals<DerivedTag>())
        condition.fluent_literals.push_back(merge_p2d(literal, derived_atom_mapping, derived_predicate_mapping, context).first.get_index());

    for (const auto numeric_constraint : element.get_numeric_constraints())
        condition.numeric_constraints.push_back(merge_p2d(numeric_constraint, context));

    ::tyr::formalism::datalog::canonicalize(condition);
    return context.destination.get_or_create(condition);
}

inline std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& fluent_atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          MergeDatalogContext& context)
{
    auto condition_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundConjunctiveCondition>();
    auto& condition = *condition_ptr;
    condition.clear();

    for (const auto fact : element.template get_facts<PositiveTag>())
        if (const auto literal = merge_p2d(fact, true, fluent_atom_mapping, fluent_predicate_mapping, context))
            condition.fluent_literals.push_back(literal->get_index());

    for (const auto fact : element.template get_facts<NegativeTag>())
        if (const auto literal = merge_p2d(fact, false, fluent_atom_mapping, fluent_predicate_mapping, context))
            condition.fluent_literals.push_back(literal->get_index());

    for (const auto numeric_constraint : element.get_numeric_constraints())
        condition.numeric_constraints.push_back(merge_p2d(numeric_constraint, context));

    ::tyr::formalism::datalog::canonicalize(condition);
    return context.destination.get_or_create(condition);
}

// Numeric

template<FactKind T>
std::pair<::tyr::formalism::datalog::FunctionView<T>, bool> merge_p2d(FunctionView<T> element, MergeDatalogContext& context)
{
    auto function_ptr = context.builder.template get_builder<::tyr::formalism::Function<T>>();
    auto& function = *function_ptr;
    function.clear();

    function.name = element.get_name();
    function.arity = element.get_arity();

    canonicalize(function);
    return context.destination.get_or_create(function);
}

template<FactKind T>
std::pair<::tyr::formalism::datalog::FunctionTermView<T>, bool> merge_p2d(FunctionTermView<T> element, MergeDatalogContext& context)
{
    auto fterm_ptr = context.builder.template get_builder<::tyr::formalism::datalog::FunctionTerm<T>>();
    auto& fterm = *fterm_ptr;
    fterm.clear();

    fterm.function = element.get_function().get_index();
    for (const auto term : element.get_terms())
        fterm.terms.push_back(merge_p2d(term, context));

    canonicalize(fterm);
    return context.destination.get_or_create(fterm);
}

template<FactKind T>
std::pair<::tyr::formalism::datalog::FunctionBindingView<T>, bool> merge_p2d(FunctionBindingView<T> element, MergeDatalogContext& context)
{
    auto binding_ptr = context.builder.template get_builder<RelationBinding<Function<T>>>();
    auto& binding = *binding_ptr;
    binding.clear();

    binding.relation = merge_p2d(element.get_relation(), context).first.get_index();
    for (const auto object : element.get_objects())
        binding.objects.push_back(object.get_index());

    canonicalize(binding);
    return context.destination.get_or_create(binding);
}

template<FactKind T>
std::pair<::tyr::formalism::datalog::GroundFunctionTermView<T>, bool> merge_p2d(GroundFunctionTermView<T> element, MergeDatalogContext& context)
{
    auto fterm_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundFunctionTerm<T>>();
    auto& fterm = *fterm_ptr;
    fterm.clear();

    fterm.binding = merge_p2d(element.get_row(), context).first.get_index();

    canonicalize(fterm);
    return context.destination.get_or_create(fterm);
}

template<FactKind T>
std::pair<::tyr::formalism::datalog::GroundFunctionTermValueView<T>, bool> merge_p2d(GroundFunctionTermValueView<T> element, MergeDatalogContext& context)
{
    auto fterm_value_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundFunctionTermValue<T>>();
    auto& fterm_value = *fterm_value_ptr;
    fterm_value.clear();

    fterm_value.fterm = merge_p2d(element.get_fterm(), context).first.get_index();
    fterm_value.value = element.get_value();

    canonicalize(fterm_value);
    return context.destination.get_or_create(fterm_value);
}

template<::tyr::formalism::NumericEffectOpKind DOp, NumericEffectOpKind Op>
std::pair<::tyr::formalism::datalog::NumericEffectView<DOp, FluentTag>, bool> merge_numeric_effect_as(NumericEffectView<Op, FluentTag> element,
                                                                                               MergeDatalogContext& context)
{
    auto numeric_effect_ptr = context.builder.template get_builder<::tyr::formalism::datalog::NumericEffect<DOp, FluentTag>>();
    auto& numeric_effect = *numeric_effect_ptr;
    numeric_effect.clear();

    numeric_effect.fterm = merge_p2d(element.get_fterm(), context).first.get_index();
    numeric_effect.fexpr = merge_p2d(element.get_fexpr(), context);

    canonicalize(numeric_effect);
    return context.destination.get_or_create(numeric_effect);
}

inline std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Assign, FluentTag>, bool> merge_p2d(NumericEffectView<Assign, FluentTag> element,
                                                                                                      MergeDatalogContext& context)
{
    return merge_numeric_effect_as<::tyr::formalism::Assign>(element, context);
}

inline std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Increase, FluentTag>, bool> merge_p2d(NumericEffectView<Increase, FluentTag> element,
                                                                                                        MergeDatalogContext& context)
{
    return merge_numeric_effect_as<::tyr::formalism::Increase>(element, context);
}

inline std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::Decrease, FluentTag>, bool> merge_p2d(NumericEffectView<Decrease, FluentTag> element,
                                                                                                        MergeDatalogContext& context)
{
    return merge_numeric_effect_as<::tyr::formalism::Decrease>(element, context);
}

inline std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::ScaleUp, FluentTag>, bool> merge_p2d(NumericEffectView<ScaleUp, FluentTag> element,
                                                                                                       MergeDatalogContext& context)
{
    return merge_numeric_effect_as<::tyr::formalism::ScaleUp>(element, context);
}

inline std::pair<::tyr::formalism::datalog::NumericEffectView<::tyr::formalism::ScaleDown, FluentTag>, bool> merge_p2d(NumericEffectView<ScaleDown, FluentTag> element,
                                                                                                         MergeDatalogContext& context)
{
    return merge_numeric_effect_as<::tyr::formalism::ScaleDown>(element, context);
}

inline ygg::Data<::tyr::formalism::datalog::NumericEffectOperator<FluentTag>> merge_p2d(NumericEffectOperatorView<FluentTag> element, MergeDatalogContext& context)
{
    using OperatorData = ygg::Data<::tyr::formalism::datalog::NumericEffectOperator<FluentTag>>;

    return visit([&](auto&& arg) { return OperatorData(typename OperatorData::Variant(merge_p2d(arg, context).first.get_index())); }, element.get_variant());
}

template<::tyr::formalism::NumericEffectOpKind DOp, NumericEffectOpKind Op>
std::pair<::tyr::formalism::datalog::GroundNumericEffectView<DOp, FluentTag>, bool>
merge_ground_numeric_effect_as(GroundNumericEffectView<Op, FluentTag> element, MergeDatalogContext& context)
{
    auto numeric_effect_ptr = context.builder.template get_builder<::tyr::formalism::datalog::GroundNumericEffect<DOp, FluentTag>>();
    auto& numeric_effect = *numeric_effect_ptr;
    numeric_effect.clear();

    numeric_effect.fterm = merge_p2d(element.get_fterm(), context).first.get_index();
    numeric_effect.fexpr = merge_p2d(element.get_fexpr(), context);

    canonicalize(numeric_effect);
    return context.destination.get_or_create(numeric_effect);
}

inline std::pair<::tyr::formalism::datalog::GroundNumericEffectView<::tyr::formalism::Assign, FluentTag>, bool>
merge_p2d(GroundNumericEffectView<Assign, FluentTag> element, MergeDatalogContext& context)
{
    return merge_ground_numeric_effect_as<::tyr::formalism::Assign>(element, context);
}

inline std::pair<::tyr::formalism::datalog::GroundNumericEffectView<::tyr::formalism::Increase, FluentTag>, bool>
merge_p2d(GroundNumericEffectView<Increase, FluentTag> element, MergeDatalogContext& context)
{
    return merge_ground_numeric_effect_as<::tyr::formalism::Increase>(element, context);
}

inline std::pair<::tyr::formalism::datalog::GroundNumericEffectView<::tyr::formalism::Decrease, FluentTag>, bool>
merge_p2d(GroundNumericEffectView<Decrease, FluentTag> element, MergeDatalogContext& context)
{
    return merge_ground_numeric_effect_as<::tyr::formalism::Decrease>(element, context);
}

inline std::pair<::tyr::formalism::datalog::GroundNumericEffectView<::tyr::formalism::ScaleUp, FluentTag>, bool>
merge_p2d(GroundNumericEffectView<ScaleUp, FluentTag> element, MergeDatalogContext& context)
{
    return merge_ground_numeric_effect_as<::tyr::formalism::ScaleUp>(element, context);
}

inline std::pair<::tyr::formalism::datalog::GroundNumericEffectView<::tyr::formalism::ScaleDown, FluentTag>, bool>
merge_p2d(GroundNumericEffectView<ScaleDown, FluentTag> element, MergeDatalogContext& context)
{
    return merge_ground_numeric_effect_as<::tyr::formalism::ScaleDown>(element, context);
}

inline ygg::Data<::tyr::formalism::datalog::GroundNumericEffectOperator<FluentTag>> merge_p2d(GroundNumericEffectOperatorView<FluentTag> element,
                                                                                               MergeDatalogContext& context)
{
    using OperatorData = ygg::Data<::tyr::formalism::datalog::GroundNumericEffectOperator<FluentTag>>;

    return visit([&](auto&& arg) { return OperatorData(typename OperatorData::Variant(merge_p2d(arg, context).first.get_index())); }, element.get_variant());
}

inline ygg::Data<::tyr::formalism::datalog::FunctionExpression> merge_p2d(FunctionExpressionView element, MergeDatalogContext& context)
{
    return visit(
        [&](auto&& arg) -> ygg::Data<::tyr::formalism::datalog::FunctionExpression>
        {
            using Alternative = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<Alternative, ygg::float_t>)
                return ygg::Data<::tyr::formalism::datalog::FunctionExpression>(arg);
            else if constexpr (std::is_same_v<Alternative, LiftedArithmeticOperatorView>)
                return ygg::Data<::tyr::formalism::datalog::FunctionExpression>(merge_p2d(arg, context));
            else if constexpr (std::is_same_v<Alternative, FunctionTermView<AuxiliaryTag>>)
                throw std::logic_error("AuxiliaryTag FunctionTerm must not be merged.");
            else
                return ygg::Data<::tyr::formalism::datalog::FunctionExpression>(merge_p2d(arg, context).first.get_index());
        },
        element.get_variant());
}

inline ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression> merge_p2d(GroundFunctionExpressionView element, MergeDatalogContext& context)
{
    return visit(
        [&](auto&& arg) -> ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>
        {
            using Alternative = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<Alternative, ygg::float_t>)
                return ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>(arg);
            else if constexpr (std::is_same_v<Alternative, GroundArithmeticOperatorView>)
                return ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>(merge_p2d(arg, context));
            else if constexpr (std::is_same_v<Alternative, GroundFunctionTermView<AuxiliaryTag>>)
                throw std::logic_error("AuxiliaryTag GroundFunctionTerm must not be merged.");
            else
                return ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>(merge_p2d(arg, context).first.get_index());
        },
        element.get_variant());
}

template<OpKind O, typename T>
std::pair<::tyr::formalism::datalog::UnaryOperatorView<O, to_datalog_payload_t<T>>, bool> merge_p2d(UnaryOperatorView<O, T> element, MergeDatalogContext& context)
{
    using T_DST = to_datalog_payload_t<T>;

    auto unary_ptr = context.builder.template get_builder<::tyr::formalism::datalog::UnaryOperator<O, T_DST>>();
    auto& unary = *unary_ptr;
    unary.clear();

    unary.arg = merge_p2d(element.get_arg(), context);

    canonicalize(unary);
    return context.destination.get_or_create(unary);
}

template<OpKind O, typename T>
std::pair<::tyr::formalism::datalog::BinaryOperatorView<O, to_datalog_payload_t<T>>, bool> merge_p2d(BinaryOperatorView<O, T> element, MergeDatalogContext& context)
{
    using T_DST = to_datalog_payload_t<T>;

    auto binary_ptr = context.builder.template get_builder<::tyr::formalism::datalog::BinaryOperator<O, T_DST>>();
    auto& binary = *binary_ptr;
    binary.clear();

    binary.lhs = merge_p2d(element.get_lhs(), context);
    binary.rhs = merge_p2d(element.get_rhs(), context);

    canonicalize(binary);
    return context.destination.get_or_create(binary);
}

template<OpKind O, typename T>
std::pair<::tyr::formalism::datalog::MultiOperatorView<O, to_datalog_payload_t<T>>, bool> merge_p2d(MultiOperatorView<O, T> element, MergeDatalogContext& context)
{
    using T_DST = to_datalog_payload_t<T>;

    auto multi_ptr = context.builder.template get_builder<::tyr::formalism::datalog::MultiOperator<O, T_DST>>();
    auto& multi = *multi_ptr;
    multi.clear();

    for (const auto arg : element.get_args())
        multi.args.push_back(merge_p2d(arg, context));

    canonicalize(multi);
    return context.destination.get_or_create(multi);
}

template<typename T>
ygg::Data<::tyr::formalism::datalog::ArithmeticOperator<to_datalog_payload_t<T>>> merge_p2d(ArithmeticOperatorView<T> element, MergeDatalogContext& context)
{
    using T_DST = to_datalog_payload_t<T>;

    return visit([&](auto&& arg) { return ygg::Data<::tyr::formalism::datalog::ArithmeticOperator<T_DST>>(merge_p2d(arg, context).first.get_index()); },
                 element.get_variant());
}

template<typename T>
ygg::Data<::tyr::formalism::datalog::BooleanOperator<to_datalog_payload_t<T>>> merge_p2d(BooleanOperatorView<T> element, MergeDatalogContext& context)
{
    using T_DST = to_datalog_payload_t<T>;

    return visit([&](auto&& arg) { return ygg::Data<::tyr::formalism::datalog::BooleanOperator<T_DST>>(merge_p2d(arg, context).first.get_index()); },
                 element.get_variant());
}

}

#ifndef TYR_HEADER_INSTANTIATION

namespace tyr::formalism::planning
{
extern template std::pair<::tyr::formalism::datalog::PredicateView<StaticTag>, bool> merge_p2d(PredicateView<StaticTag> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::PredicateView<FluentTag>, bool> merge_p2d(PredicateView<FluentTag> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::PredicateView<FluentTag>, bool> merge_p2d(PredicateView<DerivedTag> element, MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::AtomView<StaticTag>, bool>
merge_p2d(AtomView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::AtomView<FluentTag>, bool>
merge_p2d(AtomView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::AtomView<FluentTag>, bool>
merge_p2d(AtomView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::PredicateBindingView<StaticTag>, bool>
merge_p2d(PredicateBindingView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::PredicateBindingView<FluentTag>, bool>
merge_p2d(PredicateBindingView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::PredicateBindingView<FluentTag>, bool>
merge_p2d(PredicateBindingView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::GroundAtomView<StaticTag>, bool>
merge_p2d(GroundAtomView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::GroundAtomView<FluentTag>, bool>
merge_p2d(GroundAtomView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::GroundAtomView<FluentTag>, bool>
merge_p2d(GroundAtomView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::LiteralView<StaticTag>, bool>
merge_p2d(LiteralView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::LiteralView<FluentTag>, bool>
merge_p2d(LiteralView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::LiteralView<FluentTag>, bool>
merge_p2d(LiteralView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::GroundLiteralView<StaticTag>, bool>
merge_p2d(GroundLiteralView<StaticTag> element,
          const ygg::UnorderedMap<PredicateView<StaticTag>, ::tyr::formalism::datalog::PredicateView<StaticTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::GroundLiteralView<FluentTag>, bool>
merge_p2d(GroundLiteralView<FluentTag> element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::GroundLiteralView<FluentTag>, bool>
merge_p2d(GroundLiteralView<DerivedTag> element,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

std::optional<::tyr::formalism::datalog::GroundLiteralView<FluentTag>>
merge_p2d(FDRFactView<FluentTag> element,
          bool polarity,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

std::optional<::tyr::formalism::datalog::GroundLiteralView<FluentTag>>
merge_p2d(FDRFactView<FluentTag> element,
          bool polarity,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& predicate_mapping,
          MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& derived_predicate_mapping,
          MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& fluent_atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          ygg::UnorderedMap<GroundAtomView<DerivedTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& derived_atom_mapping,
          const ygg::UnorderedMap<PredicateView<DerivedTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& derived_predicate_mapping,
          MergeDatalogContext& context);

std::pair<::tyr::formalism::datalog::GroundConjunctiveConditionView, bool>
merge_p2d(GroundConjunctiveConditionView element,
          ygg::UnorderedMap<GroundAtomView<FluentTag>, ::tyr::formalism::datalog::GroundAtomView<FluentTag>>& fluent_atom_mapping,
          const ygg::UnorderedMap<PredicateView<FluentTag>, ::tyr::formalism::datalog::PredicateView<FluentTag>>& fluent_predicate_mapping,
          MergeDatalogContext& context);

// Numeric

extern template std::pair<::tyr::formalism::datalog::FunctionView<StaticTag>, bool> merge_p2d(FunctionView<StaticTag> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::FunctionView<FluentTag>, bool> merge_p2d(FunctionView<FluentTag> element, MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::FunctionTermView<StaticTag>, bool> merge_p2d(FunctionTermView<StaticTag> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::FunctionTermView<FluentTag>, bool> merge_p2d(FunctionTermView<FluentTag> element, MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::FunctionBindingView<StaticTag>, bool> merge_p2d(FunctionBindingView<StaticTag> element,
                                                                                              MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::FunctionBindingView<FluentTag>, bool> merge_p2d(FunctionBindingView<FluentTag> element,
                                                                                              MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::GroundFunctionTermView<StaticTag>, bool> merge_p2d(GroundFunctionTermView<StaticTag> element,
                                                                                                 MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::GroundFunctionTermView<FluentTag>, bool> merge_p2d(GroundFunctionTermView<FluentTag> element,
                                                                                                 MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::GroundFunctionTermValueView<StaticTag>, bool> merge_p2d(GroundFunctionTermValueView<StaticTag> element,
                                                                                                      MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::GroundFunctionTermValueView<FluentTag>, bool> merge_p2d(GroundFunctionTermValueView<FluentTag> element,
                                                                                                      MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::UnaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(UnaryOperatorView<Sub, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::UnaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(UnaryOperatorView<Sub, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Eq, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Eq, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Ne, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Ne, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Ge, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Ge, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Gt, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Gt, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Le, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Le, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Lt, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Lt, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Add, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Add, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Sub, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Mul, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Div, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Div, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Eq, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Eq, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Ne, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Ne, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Ge, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Ge, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Gt, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Gt, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Le, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Le, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Lt, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Lt, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Add, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Add, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Sub, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Sub, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Mul, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::BinaryOperatorView<Div, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(BinaryOperatorView<Div, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

extern template std::pair<::tyr::formalism::datalog::MultiOperatorView<Add, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(MultiOperatorView<Add, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::MultiOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::FunctionExpression>>, bool>
merge_p2d(MultiOperatorView<Mul, ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::MultiOperatorView<Add, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(MultiOperatorView<Add, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
extern template std::pair<::tyr::formalism::datalog::MultiOperatorView<Mul, ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>, bool>
merge_p2d(MultiOperatorView<Mul, ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

extern template ygg::Data<::tyr::formalism::datalog::ArithmeticOperator<ygg::Data<::tyr::formalism::datalog::FunctionExpression>>>
merge_p2d(ArithmeticOperatorView<ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template ygg::Data<::tyr::formalism::datalog::ArithmeticOperator<ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>>
merge_p2d(ArithmeticOperatorView<ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);

extern template ygg::Data<::tyr::formalism::datalog::BooleanOperator<ygg::Data<::tyr::formalism::datalog::FunctionExpression>>>
merge_p2d(BooleanOperatorView<ygg::Data<FunctionExpression>> element, MergeDatalogContext& context);
extern template ygg::Data<::tyr::formalism::datalog::BooleanOperator<ygg::Data<::tyr::formalism::datalog::GroundFunctionExpression>>>
merge_p2d(BooleanOperatorView<ygg::Data<GroundFunctionExpression>> element, MergeDatalogContext& context);
}

#endif

#endif
