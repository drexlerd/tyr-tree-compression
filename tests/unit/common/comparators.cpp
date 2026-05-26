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
#include <tyr/common/comparators.hpp>
#include <tyr/common/cista_comparators.hpp>
#include <tyr/common/block_array_comparators.hpp>

#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace tyr::tests
{

TEST(TyrTests, TyrCommonLessRangeMatchesContainerLess)
{
    const auto lhs = std::vector<int> { 1, 2, 3 };
    const auto rhs = std::vector<int> { 1, 2, 4 };

    EXPECT_TRUE(less_range(lhs, rhs));
    EXPECT_EQ(less_range(lhs, rhs), Less<std::vector<int>> {}(lhs, rhs));
    EXPECT_EQ(less_range(std::span<const int>(lhs), std::span<const int>(rhs)), Less<std::span<const int>> {}(std::span<const int>(lhs), std::span<const int>(rhs)));
}

TEST(TyrTests, TyrCommonLessRangeUsesElementLess)
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto lhs = std::vector<double> { nan, 1.0 };
    const auto rhs = std::vector<double> { nan, 2.0 };

    EXPECT_TRUE(less_range(lhs, rhs));
    EXPECT_TRUE(Less<std::vector<double>> {}(lhs, rhs));
}

TEST(TyrTests, TyrCommonLessRangeOrdersPrefixes)
{
    const auto lhs = std::vector<int> { 1, 2 };
    const auto rhs = std::vector<int> { 1, 2, 3 };

    EXPECT_TRUE(less_range(lhs, rhs));
    EXPECT_FALSE(less_range(rhs, lhs));
}

TEST(TyrTests, TyrCommonBlockArrayComparatorOrdersViews)
{
    auto lhs_storage = std::vector<uint8_t> { 1, 2 };
    auto rhs_storage = std::vector<uint8_t> { 1, 3 };

    using View = BasicBlockArrayView<uint8_t, bit::ForwardingBlockCoder<uint8_t>>;
    auto lhs = View(lhs_storage.data(), lhs_storage.size());
    auto rhs = View(rhs_storage.data(), rhs_storage.size());

    EXPECT_TRUE(Less<View> {}(lhs, rhs));
    EXPECT_FALSE(Less<View> {}(rhs, lhs));
}

TEST(TyrTests, TyrCommonCistaLessAdaptersOrderOffsetVector)
{
    auto lhs = ::cista::offset::vector<int> {};
    lhs.emplace_back(1);
    lhs.emplace_back(2);

    auto rhs = ::cista::offset::vector<int> {};
    rhs.emplace_back(1);
    rhs.emplace_back(3);

    EXPECT_TRUE(Less<::cista::offset::vector<int>> {}(lhs, rhs));
    EXPECT_FALSE(Less<::cista::offset::vector<int>> {}(rhs, lhs));
}

}
