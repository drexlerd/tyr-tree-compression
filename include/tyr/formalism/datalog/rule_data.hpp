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

#ifndef TYR_FORMALISM_DATALOG_RULE_DATA_HPP_
#define TYR_FORMALISM_DATALOG_RULE_DATA_HPP_

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/containers/variant.hpp>
#include "tyr/formalism/datalog/atom_index.hpp"
#include "tyr/formalism/datalog/conjunctive_condition_index.hpp"
#include "tyr/formalism/datalog/declarations.hpp"
#include "tyr/formalism/datalog/numeric_effect_operator_data.hpp"
#include "tyr/formalism/datalog/rule_index.hpp"
#include "tyr/formalism/variable_index.hpp"

namespace ygg
{
using namespace ::tyr;

template<>
struct Data<::tyr::formalism::datalog::Rule>
{
    using Head = ::cista::offset::variant<ygg::Index<::tyr::formalism::datalog::Atom<::tyr::formalism::FluentTag>>,
                                         ygg::Data<::tyr::formalism::datalog::NumericEffectOperator<::tyr::formalism::FluentTag>>>;

    ygg::Index<::tyr::formalism::datalog::Rule> index;
    ygg::IndexList<::tyr::formalism::Variable> variables;
    ygg::Index<::tyr::formalism::datalog::ConjunctiveCondition> body;
    Head head;
    ygg::uint_t cost;

    Data() = default;
    Data(ygg::Index<::tyr::formalism::datalog::Rule> index,
         ygg::IndexList<::tyr::formalism::Variable> variables,
         ygg::Index<::tyr::formalism::datalog::ConjunctiveCondition> body,
         Head head,
         ygg::uint_t cost) :
        index(index),
        variables(std::move(variables)),
        body(body),
        head(head),
        cost(cost)
    {
    }
    Data(const Data& other) = delete;
    Data& operator=(const Data& other) = delete;
    Data(Data&& other) = default;
    Data& operator=(Data&& other) = default;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(variables);
        ygg::clear(body);
        ygg::clear(head);
        ygg::clear(cost);
    }

    auto cista_members() const noexcept { return std::tie(index, variables, body, head, cost); }
    auto identifying_members() const noexcept { return std::tie(variables, body, head, cost); }
};

static_assert(!ygg::uses_trivial_storage_v<::tyr::formalism::datalog::Rule>);
}

#endif
