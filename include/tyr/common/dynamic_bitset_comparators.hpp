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

#include "tyr/common/comparators.hpp"
#include "tyr/common/dynamic_bitset.hpp"
#include "tyr/common/equal_to.hpp"
#include "tyr/common/hash.hpp"

#include <concepts>
#include <cstddef>
#include <iterator>
#include <vector>

namespace tyr
{

template<typename Block, typename Allocator>
struct Less<boost::dynamic_bitset<Block, Allocator>>
{
    using Type = boost::dynamic_bitset<Block, Allocator>;

    bool operator()(const Type& lhs, const Type& rhs) const
    {
        if (lhs.size() != rhs.size())
            return lhs.size() < rhs.size();

        auto lhs_blocks = std::vector<Block>();
        auto rhs_blocks = std::vector<Block>();
        lhs_blocks.reserve(lhs.num_blocks());
        rhs_blocks.reserve(rhs.num_blocks());
        boost::to_block_range(lhs, std::back_inserter(lhs_blocks));
        boost::to_block_range(rhs, std::back_inserter(rhs_blocks));

        return less_range(lhs_blocks, rhs_blocks);
    }
};

template<typename Block, typename Allocator>
struct Hash<boost::dynamic_bitset<Block, Allocator>>
{
    using Type = boost::dynamic_bitset<Block, Allocator>;

    size_t operator()(const Type& bitset) const
    {
        auto blocks = std::vector<Block>();
        blocks.reserve(bitset.num_blocks());
        boost::to_block_range(bitset, std::back_inserter(blocks));

        size_t seed = bitset.size();
        for (const auto& block : blocks)
            hash_combine(seed, block);
        return seed;
    }
};

template<typename Block, typename Allocator>
struct EqualTo<boost::dynamic_bitset<Block, Allocator>>
{
    using Type = boost::dynamic_bitset<Block, Allocator>;

    bool operator()(const Type& lhs, const Type& rhs) const { return lhs == rhs; }
};

template<std::unsigned_integral Block>
struct Less<BitsetSpan<Block>>
{
    bool operator()(const BitsetSpan<Block>& lhs, const BitsetSpan<Block>& rhs) const noexcept
    {
        assert(lhs.trailing_bits_zero());
        assert(rhs.trailing_bits_zero());

        if (lhs.num_bits() != rhs.num_bits())
            return lhs.num_bits() < rhs.num_bits();

        return less_range(lhs.blocks(), rhs.blocks());
    }
};

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
