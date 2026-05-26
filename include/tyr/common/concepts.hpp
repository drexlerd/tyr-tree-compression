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

#ifndef TYR_COMMON_CONCEPTS_HPP_
#define TYR_COMMON_CONCEPTS_HPP_

#include <concepts>
#include <ranges>
#include <type_traits>

namespace tyr
{

/// @brief Check whether T has a function that returns members that aims to identify the class.
template<typename T>
concept Identifiable = requires(const T a) {
    { a.identifying_members() };
};

template<typename Range, typename Value>
concept InputRangeOf = std::ranges::input_range<Range> && std::same_as<std::ranges::range_value_t<Range>, Value>;

template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

template<typename T, typename U>
concept SameAsIgnoringCvref = std::same_as<std::remove_cvref_t<T>, U>;

template<typename T, typename U>
concept UnsignedIntegralSameAsIgnoringConst = std::unsigned_integral<std::remove_const_t<T>> && std::same_as<std::remove_const_t<T>, std::remove_const_t<U>>;

template<typename T>
struct dependent_false : std::false_type
{
};

}

#endif
