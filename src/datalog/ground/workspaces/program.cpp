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

#include <stdexcept>
#include <type_traits>
#include <yggdrasil/containers/variant.hpp>

namespace f = tyr::formalism;
namespace fd = tyr::formalism::datalog;

namespace tyr::datalog
{
namespace
{
void reject_if_function_facts(fd::GroundProgramView program)
{
    if (!program.template get_fterm_values<f::StaticTag>().empty() || !program.template get_fterm_values<f::FluentTag>().empty())
        throw std::invalid_argument("ground queue solver does not support function facts");
}

void reject_if_numeric_constraints(fd::GroundConjunctiveConditionView condition)
{
    if (!condition.get_numeric_constraints().empty())
        throw std::invalid_argument("ground queue solver does not support numeric constraints");
}
}

ConstProgramWorkspace<GroundTag>::ConstProgramWorkspace(fd::GroundProgramView program_) : program(program_), fluent_precondition_to_rules()
{
    reject_if_function_facts(program);

    for (const auto rule : program.get_ground_rules())
    {
        const auto body = rule.get_body();
        reject_if_numeric_constraints(body);

        ygg::visit(
            [](auto&& head)
            {
                using Head = std::decay_t<decltype(head)>;
                if constexpr (!std::is_same_v<Head, fd::GroundAtomView<f::FluentTag>>)
                    throw std::invalid_argument("ground queue solver only supports fluent atom heads");
            },
            rule.get_head());

        for (const auto literal : body.template get_literals<f::FluentTag>())
            fluent_precondition_to_rules[literal.get_atom()].push_back(rule);
    }
}

}