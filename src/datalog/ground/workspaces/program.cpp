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

#include "tyr/datalog/ground/workspaces/program.hpp"

#include <type_traits>
#include <yggdrasil/containers/variant.hpp>

namespace f = tyr::formalism;
namespace fd = tyr::formalism::datalog;

namespace tyr::datalog
{
namespace
{
void collect_fluent_terms(ygg::float_t, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>&) {}

void collect_fluent_terms(fd::GroundFunctionTermView<f::StaticTag>, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>&) {}

void collect_fluent_terms(fd::GroundFunctionTermView<f::FluentTag> term, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    terms.insert(term);
}

void collect_fluent_terms(fd::GroundFunctionExpressionView expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms);

void collect_fluent_terms(fd::GroundArithmeticOperatorView expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms);

template<f::ArithmeticOpKind O>
void collect_fluent_terms(fd::GroundUnaryOperatorView<O> expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    collect_fluent_terms(expression.get_arg(), terms);
}

template<f::ArithmeticOpKind O>
void collect_fluent_terms(fd::GroundBinaryOperatorView<O> expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    collect_fluent_terms(expression.get_lhs(), terms);
    collect_fluent_terms(expression.get_rhs(), terms);
}

template<f::ArithmeticOpKind O>
void collect_fluent_terms(fd::GroundMultiOperatorView<O> expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    for (const auto child : expression.get_args())
        collect_fluent_terms(child, terms);
}

template<f::BooleanOpKind O>
void collect_fluent_terms(fd::GroundBinaryOperatorView<O> expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    collect_fluent_terms(expression.get_lhs(), terms);
    collect_fluent_terms(expression.get_rhs(), terms);
}

void collect_fluent_terms(fd::GroundFunctionExpressionView expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    ygg::visit([&](auto&& arg) { collect_fluent_terms(arg, terms); }, expression.get_variant());
}

void collect_fluent_terms(fd::GroundArithmeticOperatorView expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    ygg::visit([&](auto&& arg) { collect_fluent_terms(arg, terms); }, expression.get_variant());
}

void collect_fluent_terms(fd::GroundBooleanOperatorView expression, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    ygg::visit([&](auto&& arg) { collect_fluent_terms(arg, terms); }, expression.get_variant());
}

void collect_fluent_terms(fd::GroundNumericEffectOperatorView<f::FluentTag> effect, ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>& terms)
{
    ygg::visit(
        [&](auto&& concrete_effect)
        {
            terms.insert(concrete_effect.get_fterm());
            collect_fluent_terms(concrete_effect.get_fexpr(), terms);
        },
        effect.get_variant());
}
}

ConstProgramWorkspace<GroundTag>::ConstProgramWorkspace(fd::ProgramView<GroundTag> program_) :
    program(program_),
    fluent_precondition_to_rules(),
    fluent_function_term_to_rules()
{
    for (const auto rule : program.get_ground_rules())
    {
        const auto body = rule.get_body();
        for (const auto literal : body.template get_literals<f::FluentTag>())
            if (literal.get_polarity())
                fluent_precondition_to_rules[literal.get_atom()].push_back(rule);

        auto fluent_terms = ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>>();
        for (const auto numeric_constraint : body.get_numeric_constraints())
            collect_fluent_terms(numeric_constraint, fluent_terms);
        ygg::visit(
            [&](auto&& head)
            {
                using Head = std::decay_t<decltype(head)>;
                if constexpr (std::is_same_v<Head, fd::GroundNumericEffectOperatorView<f::FluentTag>>)
                    collect_fluent_terms(head, fluent_terms);
            },
            rule.get_head());

        for (const auto term : fluent_terms)
            fluent_function_term_to_rules[term].push_back(rule);
    }
}

}