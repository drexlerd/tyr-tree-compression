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

#ifndef TYR_FORMALISM_DATALOG_FUNCTION_EXPRESSION_DATA_HPP_
#define TYR_FORMALISM_DATALOG_FUNCTION_EXPRESSION_DATA_HPP_

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include "tyr/formalism/datalog/arithmetic_operator_data.hpp"
#include "tyr/formalism/datalog/declarations.hpp"
#include "tyr/formalism/datalog/function_term_index.hpp"

namespace ygg
{
using namespace ::tyr;

template<>
struct Data<::tyr::formalism::datalog::FunctionExpression>
{
    using Variant = ::cista::offset::variant<ygg::float_t,
                                             ygg::Data<::tyr::formalism::datalog::ArithmeticOperator<ygg::Data<::tyr::formalism::datalog::FunctionExpression>>>,
                                             ygg::Index<::tyr::formalism::datalog::FunctionTerm<::tyr::formalism::StaticTag>>,
                                             ygg::Index<::tyr::formalism::datalog::FunctionTerm<::tyr::formalism::FluentTag>>>;

    Variant value;

    Data() = default;
    Data(Variant value) : value(value) {}
    Data(const Data& other) = default;
    Data& operator=(const Data& other) = default;
    Data(Data&& other) = default;
    Data& operator=(Data&& other) = default;

    void clear() noexcept { ygg::clear(value); }

    auto cista_members() const noexcept { return std::tie(value); }
    auto identifying_members() const noexcept { return std::tie(value); }
};

static_assert(!ygg::uses_trivial_storage_v<::tyr::formalism::datalog::FunctionExpression>);

}

#endif
