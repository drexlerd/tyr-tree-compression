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

#include <gtest/gtest.h>
#include <tyr/common/hash.hpp>
#include <tyr/common/comparators.hpp>
#include <tyr/common/associative_containers.hpp>
#include <tyr/common/dynamic_bitset_hash.hpp>
#include <tyr/common/cista_hash.hpp>
#include <tyr/common/observer_ptr_comparators.hpp>
#include <tyr/common/observer_ptr_hash.hpp>

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace tyr::tests
{

TEST(TyrTests, TyrCommonHashRangeMatchesContainerHash)
{
    const auto values = std::vector<int> { 1, 2, 3 };

    EXPECT_EQ(hash_range(values), Hash<std::vector<int>> {}(values));
    EXPECT_EQ(hash_range(std::span<const int>(values)), Hash<std::span<const int>> {}(std::span<const int>(values)));
}

TEST(TyrTests, TyrCommonHashRangeKeepsSizeInSeed)
{
    const auto one = std::array<int, 1> { 0 };
    const auto two = std::array<int, 2> { 0, 0 };

    EXPECT_NE(hash_range(one), hash_range(two));
}

TEST(TyrTests, TyrCommonCistaHashAdaptersHashOffsetVector)
{
    auto values = ::cista::offset::vector<int> {};
    values.emplace_back(1);
    values.emplace_back(2);

    EXPECT_EQ(hash_range(values), Hash<::cista::offset::vector<int>> {}(values));
}

TEST(TyrTests, TyrCommonCistaHashAdaptersHashOffsetStringOptionalAndVariant)
{
    auto lhs_string = ::cista::offset::string {};
    auto rhs_string = ::cista::offset::string {};
    lhs_string = "alpha";
    rhs_string = "alpha";
    EXPECT_EQ(Hash<::cista::offset::string> {}(lhs_string), Hash<::cista::offset::string> {}(rhs_string));

    auto lhs_optional = ::cista::optional<int> { 7 };
    auto rhs_optional = ::cista::optional<int> { 7 };
    auto empty_optional = ::cista::optional<int> {};
    EXPECT_EQ(Hash<::cista::optional<int>> {}(lhs_optional), Hash<::cista::optional<int>> {}(rhs_optional));
    EXPECT_NE(Hash<::cista::optional<int>> {}(lhs_optional), Hash<::cista::optional<int>> {}(empty_optional));

    using Variant = ::cista::offset::variant<int, unsigned>;
    auto lhs_variant = Variant { 9 };
    auto rhs_variant = Variant { 9 };
    EXPECT_EQ(Hash<Variant> {}(lhs_variant), Hash<Variant> {}(rhs_variant));
}

TEST(TyrTests, TyrCommonObserverPtrHashAdaptersHashPointee)
{
    const auto value = 7;
    const auto ptr = make_observer(value);

    EXPECT_EQ(Hash<int> {}(value), Hash<ObserverPtr<const int>> {}(ptr));
}

TEST(TyrTests, TyrCommonHashAdaptersHashTyrOrderedAssociativeAliases)
{
    const auto set = Set<int> { 1, 2 };
    EXPECT_EQ(hash_range(set), Hash<Set<int>> {}(set));

    const auto map = Map<int, int> { { 1, 2 } };
    EXPECT_EQ(hash_range(map), (Hash<Map<int, int>> {}(map)));
}

TEST(TyrTests, TyrCommonDynamicBitsetHashAdaptersHashBoostDynamicBitsets)
{
    auto lhs = boost::dynamic_bitset<>(8);
    auto rhs = boost::dynamic_bitset<>(8);

    lhs.set(1);
    rhs.set(1);

    EXPECT_EQ(Hash<boost::dynamic_bitset<>> {}(lhs), Hash<boost::dynamic_bitset<>> {}(rhs));

    rhs.set(2);

    EXPECT_NE(Hash<boost::dynamic_bitset<>> {}(lhs), Hash<boost::dynamic_bitset<>> {}(rhs));
}

TEST(TyrTests, TyrCommonDynamicBitsetHashAdaptersHashBitsetSpans)
{
    const auto lhs_blocks = std::vector<std::uint64_t> { 0b1010 };
    const auto rhs_blocks = std::vector<std::uint64_t> { 0b1010 };
    const auto different_blocks = std::vector<std::uint64_t> { 0b0010 };

    const auto lhs = BitsetSpan<const std::uint64_t>(lhs_blocks.data(), 4);
    const auto rhs = BitsetSpan<const std::uint64_t>(rhs_blocks.data(), 4);
    const auto different = BitsetSpan<const std::uint64_t>(different_blocks.data(), 4);

    EXPECT_EQ(Hash<BitsetSpan<const std::uint64_t>> {}(lhs), Hash<BitsetSpan<const std::uint64_t>> {}(rhs));
    EXPECT_NE(Hash<BitsetSpan<const std::uint64_t>> {}(lhs), Hash<BitsetSpan<const std::uint64_t>> {}(different));
}

}
