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

#include "tyr/formalism/datalog/expression_properties.hpp"

#include <type_traits>
#include <yggdrasil/containers/variant.hpp>

namespace f = tyr::formalism;
namespace fd = tyr::formalism::datalog;

namespace tyr::datalog
{

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
            fd::collect_fterms<f::FluentTag>(numeric_constraint, fluent_terms);
        ygg::visit(
            [&](auto&& head)
            {
                using Head = std::decay_t<decltype(head)>;
                if constexpr (std::is_same_v<Head, fd::GroundNumericEffectOperatorView<f::FluentTag>>)
                    fd::collect_fterms<f::FluentTag>(head, fluent_terms);
            },
            rule.get_head());

        for (const auto term : fluent_terms)
            fluent_function_term_to_rules[term].push_back(rule);
    }
}

}