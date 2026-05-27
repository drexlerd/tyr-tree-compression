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

#ifndef TYR_COMMON_ASSOCIATIVE_CONTAINER_FORMATTERS_HPP_
#define TYR_COMMON_ASSOCIATIVE_CONTAINER_FORMATTERS_HPP_

#include "tyr/common/config.hpp"
#include <type_traits>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtl/btree.hpp>
#include <gtl/phmap.hpp>

namespace tyr
{
template<typename T>
struct EqualTo;

template<typename T>
struct Hash;
}

#if TYR_ENABLE_FMT_FORMATTERS
namespace tyr::detail
{

template<typename OutputIt, typename Range>
OutputIt format_elements(OutputIt out, const Range& value)
{
    auto first = true;
    for (const auto& element : value)
    {
        if (!first)
            out = fmt::format_to(out, ", ");
        first = false;
        out = fmt::format_to(out, "{}", element);
    }
    return out;
}

template<typename OutputIt, typename Range>
OutputIt format_key_value_elements(OutputIt out, const Range& value)
{
    auto first = true;
    for (const auto& [key, mapped] : value)
    {
        if (!first)
            out = fmt::format_to(out, ", ");
        first = false;
        out = fmt::format_to(out, "{}: {}", key, mapped);
    }
    return out;
}

}

namespace fmt
{
template<typename K, typename V, typename Allocator, typename Char>
struct range_format_kind<gtl::flat_hash_map<K, V, tyr::Hash<K>, tyr::EqualTo<K>, Allocator>, Char, void> : std::false_type
{
};

template<typename K, typename C, typename A, typename Char>
struct range_format_kind<gtl::btree_set<K, C, A>, Char, void> : std::false_type
{
};

template<typename K, typename V, typename C, typename A, typename Char>
struct range_format_kind<gtl::btree_map<K, V, C, A>, Char, void> : std::false_type
{
};

template<typename K, typename V, typename Allocator>
struct formatter<gtl::flat_hash_map<K, V, tyr::Hash<K>, tyr::EqualTo<K>, Allocator>, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const gtl::flat_hash_map<K, V, tyr::Hash<K>, tyr::EqualTo<K>, Allocator>& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), "{{");
        out = tyr::detail::format_key_value_elements(out, value);
        return fmt::format_to(out, "}}");
    }
};

template<typename K, typename C, typename A>
struct formatter<gtl::btree_set<K, C, A>, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const gtl::btree_set<K, C, A>& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), "{{");
        out = tyr::detail::format_elements(out, value);
        return fmt::format_to(out, "}}");
    }
};

template<typename K, typename V, typename C, typename A>
struct formatter<gtl::btree_map<K, V, C, A>, char>
{
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template<typename FormatContext>
    auto format(const gtl::btree_map<K, V, C, A>& value, FormatContext& ctx) const
    {
        auto out = fmt::format_to(ctx.out(), "{{");
        out = tyr::detail::format_key_value_elements(out, value);
        return fmt::format_to(out, "}}");
    }
};
}
#endif

#endif
