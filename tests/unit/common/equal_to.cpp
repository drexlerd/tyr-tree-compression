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
#include <tyr/common/cista_comparators.hpp>
#include <tyr/common/observer_ptr_comparators.hpp>

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

TEST(TyrTests, TyrCommonObserverPtrEqualToAdaptersComparePointees)
{
    const auto lhs_value = 7;
    const auto rhs_value = 7;
    const auto lhs = make_observer(lhs_value);
    const auto rhs = make_observer(rhs_value);

    EXPECT_TRUE(EqualTo<ObserverPtr<const int>> {}(lhs, rhs));
}

}
