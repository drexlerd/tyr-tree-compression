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
#include <tyr/common/closed_interval.hpp>

#include <sstream>
#include <tuple>

namespace tyr::tests
{

TEST(TyrTests, TyrCommonClosedIntervalProvidesBasicIntervalOperations)
{
    const auto first = ClosedInterval<double>(1.0, 3.0);
    const auto second = ClosedInterval<double>(2.0, 4.0);

    EXPECT_FALSE(empty(first));
    EXPECT_EQ(lower(first), 1.0);
    EXPECT_EQ(upper(first), 3.0);

    const auto intersection = intersect(first, second);
    EXPECT_EQ(lower(intersection), 2.0);
    EXPECT_EQ(upper(intersection), 3.0);

    const auto envelope = hull(first, second);
    EXPECT_EQ(lower(envelope), 1.0);
    EXPECT_EQ(upper(envelope), 4.0);
    EXPECT_TRUE(subset(intersection, envelope));

    EXPECT_EQ(first.identifying_members(), std::make_tuple(false, 1.0, 3.0));

    auto out = std::ostringstream();
    out << first;
    EXPECT_EQ(out.str(), "[1,3]");
}

TEST(TyrTests, TyrCommonClosedIntervalCanonicalizesEmptyIdentityMembers)
{
    const auto first = ClosedInterval<double>();
    const auto second = ClosedInterval<double>();

    EXPECT_TRUE(empty(first));
    EXPECT_EQ(first, second);
    EXPECT_EQ(first.identifying_members(), std::make_tuple(true, 0.0, 0.0));
}

}
