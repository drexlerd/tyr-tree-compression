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
#include <tyr/common/equal_to.hpp>
#include <tyr/common/comparators.hpp>
#include <tyr/common/associative_containers.hpp>
#include <tyr/common/dynamic_bitset_equal_to.hpp>
#include <tyr/common/cista_equal_to.hpp>
#include <tyr/common/observer_ptr_comparators.hpp>
#include <tyr/common/observer_ptr_equal_to.hpp>

#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace tyr::tests
{

TEST(TyrTests, TyrCommonEqualRangeMatchesContainerEqualTo)
{
    const auto lhs = std::vector<int> { 1, 2, 3 };
    const auto rhs = std::vector<int> { 1, 2, 3 };

    EXPECT_TRUE(equal_range(lhs, rhs));
    EXPECT_EQ(equal_range(lhs, rhs), EqualTo<std::vector<int>> {}(lhs, rhs));
    EXPECT_EQ(equal_range(std::span<const int>(lhs), std::span<const int>(rhs)), EqualTo<std::span<const int>> {}(std::span<const int>(lhs), std::span<const int>(rhs)));
}

TEST(TyrTests, TyrCommonEqualRangeUsesElementEqualTo)
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto lhs = std::vector<double> { nan };
    const auto rhs = std::vector<double> { nan };

    EXPECT_TRUE(equal_range(lhs, rhs));
    EXPECT_TRUE(EqualTo<std::vector<double>> {}(lhs, rhs));
}

TEST(TyrTests, TyrCommonEqualRangeChecksSize)
{
    const auto lhs = std::vector<int> { 1, 2 };
    const auto rhs = std::vector<int> { 1, 2, 3 };

    EXPECT_FALSE(equal_range(lhs, rhs));
}

TEST(TyrTests, TyrCommonCistaEqualToAdaptersCompareOffsetVector)
{
    auto lhs = ::cista::offset::vector<int> {};
    lhs.emplace_back(1);
    lhs.emplace_back(2);

    auto rhs = ::cista::offset::vector<int> {};
    rhs.emplace_back(1);
    rhs.emplace_back(2);

    EXPECT_TRUE(EqualTo<::cista::offset::vector<int>> {}(lhs, rhs));
}

TEST(TyrTests, TyrCommonCistaEqualToAdaptersCompareOffsetStringOptionalAndVariant)
{
    auto lhs_string = ::cista::offset::string {};
    auto rhs_string = ::cista::offset::string {};
    auto different_string = ::cista::offset::string {};
    lhs_string = "alpha";
    rhs_string = "alpha";
    different_string = "beta";
    EXPECT_TRUE(EqualTo<::cista::offset::string> {}(lhs_string, rhs_string));
    EXPECT_FALSE(EqualTo<::cista::offset::string> {}(lhs_string, different_string));

    auto lhs_optional = ::cista::optional<int> { 7 };
    auto rhs_optional = ::cista::optional<int> { 7 };
    auto different_optional = ::cista::optional<int> { 8 };
    auto empty_optional = ::cista::optional<int> {};
    EXPECT_TRUE(EqualTo<::cista::optional<int>> {}(lhs_optional, rhs_optional));
    EXPECT_FALSE(EqualTo<::cista::optional<int>> {}(lhs_optional, different_optional));
    EXPECT_FALSE(EqualTo<::cista::optional<int>> {}(lhs_optional, empty_optional));

    using Variant = ::cista::offset::variant<int, unsigned>;
    auto lhs_variant = Variant { 9 };
    auto rhs_variant = Variant { 9 };
    auto different_value_variant = Variant { 10 };
    auto different_type_variant = Variant { 9U };
    EXPECT_TRUE(EqualTo<Variant> {}(lhs_variant, rhs_variant));
    EXPECT_FALSE(EqualTo<Variant> {}(lhs_variant, different_value_variant));
    EXPECT_FALSE(EqualTo<Variant> {}(lhs_variant, different_type_variant));
}

TEST(TyrTests, TyrCommonObserverPtrEqualToAdaptersComparePointees)
{
    const auto lhs_value = 7;
    const auto rhs_value = 7;
    const auto lhs = make_observer(lhs_value);
    const auto rhs = make_observer(rhs_value);

    EXPECT_TRUE(EqualTo<ObserverPtr<const int>> {}(lhs, rhs));
}

TEST(TyrTests, TyrCommonEqualToAdaptersCompareTyrOrderedAssociativeAliases)
{
    const auto lhs_set = Set<int> { 1, 2 };
    const auto rhs_set = Set<int> { 1, 2 };
    const auto different_set = Set<int> { 1, 3 };
    EXPECT_TRUE(EqualTo<Set<int>> {}(lhs_set, rhs_set));
    EXPECT_FALSE(EqualTo<Set<int>> {}(lhs_set, different_set));

    const auto lhs_map = Map<int, int> { { 1, 2 } };
    const auto rhs_map = Map<int, int> { { 1, 2 } };
    const auto different_map = Map<int, int> { { 1, 3 } };
    EXPECT_TRUE((EqualTo<Map<int, int>> {}(lhs_map, rhs_map)));
    EXPECT_FALSE((EqualTo<Map<int, int>> {}(lhs_map, different_map)));
}

TEST(TyrTests, TyrCommonDynamicBitsetEqualToAdaptersCompareBoostDynamicBitsets)
{
    auto lhs = boost::dynamic_bitset<>(8);
    auto rhs = boost::dynamic_bitset<>(8);

    lhs.set(1);
    rhs.set(1);

    EXPECT_TRUE(EqualTo<boost::dynamic_bitset<>> {}(lhs, rhs));

    rhs.set(2);

    EXPECT_FALSE(EqualTo<boost::dynamic_bitset<>> {}(lhs, rhs));
}

TEST(TyrTests, TyrCommonDynamicBitsetEqualToAdaptersCompareBitsetSpans)
{
    const auto lhs_blocks = std::vector<std::uint64_t> { 0b1010 };
    const auto rhs_blocks = std::vector<std::uint64_t> { 0b1010 };
    const auto different_blocks = std::vector<std::uint64_t> { 0b0010 };

    const auto lhs = BitsetSpan<const std::uint64_t>(lhs_blocks.data(), 4);
    const auto rhs = BitsetSpan<const std::uint64_t>(rhs_blocks.data(), 4);
    const auto different = BitsetSpan<const std::uint64_t>(different_blocks.data(), 4);

    EXPECT_TRUE(EqualTo<BitsetSpan<const std::uint64_t>> {}(lhs, rhs));
    EXPECT_FALSE(EqualTo<BitsetSpan<const std::uint64_t>> {}(lhs, different));
}

}
