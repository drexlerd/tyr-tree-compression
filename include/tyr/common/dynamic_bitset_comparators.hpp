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

#ifndef TYR_COMMON_DYNAMIC_BITSET_COMPARATORS_HPP_
#define TYR_COMMON_DYNAMIC_BITSET_COMPARATORS_HPP_

#include "tyr/common/dynamic_bitset.hpp"
#include "tyr/common/equal_to.hpp"
#include "tyr/common/hash.hpp"

#include <concepts>
#include <cstddef>

namespace tyr
{
template<std::unsigned_integral Block>
struct Hash<BitsetSpan<Block>>
{
    size_t operator()(const BitsetSpan<Block>& bitset_span) const noexcept
    {
        size_t aggregated_hash = bitset_span.num_bits();
        for (const auto& block : bitset_span.blocks())
            hash_combine(aggregated_hash, block);
        return aggregated_hash;
    }
};

template<std::unsigned_integral Block>
struct EqualTo<BitsetSpan<Block>>
{
    bool operator()(const BitsetSpan<Block>& lhs, const BitsetSpan<Block>& rhs) const noexcept { return lhs == rhs; }
};
}

#endif
