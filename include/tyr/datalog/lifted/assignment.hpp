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

#ifndef TYR_DATALOG_ASSIGNMENT_HPP_
#define TYR_DATALOG_ASSIGNMENT_HPP_

#include "tyr/formalism/object_index.hpp"
#include "tyr/formalism/parameter_index.hpp"

#include <limits>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/core/types.hpp>

namespace tyr::datalog
{

/**
 * EmptyAssignment
 */

struct EmptyAssignment
{
    static constexpr ygg::uint_t rank = 0;
};

/**
 * VertexAssignment
 */

struct VertexAssignment
{
    ::tyr::formalism::ParameterIndex index;
    ygg::Index<::tyr::formalism::Object> object;

    VertexAssignment() : index(), object() {}

    VertexAssignment(::tyr::formalism::ParameterIndex index, ygg::Index<::tyr::formalism::Object> object) : index(index), object(object) {}

    inline bool is_valid() const noexcept { return index != ::tyr::formalism::ParameterIndex::max() && object != ygg::Index<::tyr::formalism::Object>::max(); }
};

/**
 * EdgeAssignment
 */

/// @brief Encapsulate assignment of objects to variables of atoms.
struct EdgeAssignment
{
    ::tyr::formalism::ParameterIndex first_index;
    ygg::Index<::tyr::formalism::Object> first_object;
    ::tyr::formalism::ParameterIndex second_index;
    ygg::Index<::tyr::formalism::Object> second_object;

    EdgeAssignment() : first_index(), first_object(), second_index(), second_object() {}

    EdgeAssignment(::tyr::formalism::ParameterIndex first_index,
                   ygg::Index<::tyr::formalism::Object> first_object,
                   ::tyr::formalism::ParameterIndex second_index,
                   ygg::Index<::tyr::formalism::Object> second_object) :
        first_index(first_index),
        first_object(first_object),
        second_index(second_index),
        second_object(second_object)
    {
    }

    inline bool is_valid() const noexcept
    {
        return (first_index < second_index) && (first_index != ::tyr::formalism::ParameterIndex::max())
               && (second_index != ::tyr::formalism::ParameterIndex::max()) && (first_object != ygg::Index<::tyr::formalism::Object>::max())
               && (second_object != ygg::Index<::tyr::formalism::Object>::max());
    }
};
}

#endif