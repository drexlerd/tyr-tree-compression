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

#ifndef TYR_FORMALISM_DATALOG_GROUND_RULE_DATA_HPP_
#define TYR_FORMALISM_DATALOG_GROUND_RULE_DATA_HPP_

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/containers/vector.hpp>
#include "tyr/formalism/binding_index.hpp"
#include "tyr/formalism/datalog/boolean_operator_data.hpp"
#include "tyr/formalism/datalog/declarations.hpp"
#include "tyr/formalism/datalog/ground_atom_index.hpp"
#include "tyr/formalism/datalog/ground_conjunctive_condition_index.hpp"
#include "tyr/formalism/datalog/ground_literal_index.hpp"
#include "tyr/formalism/datalog/ground_numeric_effect_operator_data.hpp"
#include "tyr/formalism/datalog/ground_rule_index.hpp"
#include "tyr/formalism/datalog/rule_index.hpp"

namespace ygg
{
using namespace ::tyr;

template<>
struct Data<::tyr::formalism::datalog::GroundRule>
{
    using Head = ::cista::offset::variant<ygg::Index<::tyr::formalism::datalog::GroundAtom<::tyr::formalism::FluentTag>>,
                                         ygg::Data<::tyr::formalism::datalog::GroundNumericEffectOperator<::tyr::formalism::FluentTag>>>;

    ygg::Index<::tyr::formalism::datalog::GroundRule> index;
    ygg::Index<::tyr::formalism::RelationBinding<::tyr::formalism::datalog::Rule>> binding;
    ygg::Index<::tyr::formalism::datalog::GroundConjunctiveCondition> body;
    Head head;

    Data() = default;
    Data(ygg::Index<::tyr::formalism::datalog::GroundRule> index,
         ygg::Index<::tyr::formalism::RelationBinding<::tyr::formalism::datalog::Rule>> binding,
         ygg::Index<::tyr::formalism::datalog::GroundConjunctiveCondition> body,
         Head head) :
        index(index),
        binding(binding),
        body(body),
        head(head)
    {
    }
    Data(const Data& other) = default;
    Data& operator=(const Data& other) = default;
    Data(Data&& other) = default;
    Data& operator=(Data&& other) = default;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(binding);
        ygg::clear(body);
        ygg::clear(head);
    }

    auto cista_members() const noexcept { return std::tie(index, binding, body, head); }
    auto identifying_members() const noexcept { return std::tie(binding); }
};

static_assert(!ygg::uses_trivial_storage_v<::tyr::formalism::datalog::GroundRule>);

}

#endif
