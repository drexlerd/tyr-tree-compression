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
#include <tyr/common/associative_container_formatters.hpp>
#include <tyr/common/associative_containers.hpp>
#include <tyr/common/formatter.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tyr::tests
{

TEST(TyrTests, TyrCommonToStringUsesFmtFormatting)
{
    EXPECT_EQ(to_string(42), "42");
}

TEST(TyrTests, TyrCommonToStringsFormatsRangeElements)
{
    const auto values = std::vector<int> { 1, 2, 3 };
    const auto strings = to_strings(values);

    EXPECT_EQ(strings, (std::vector<std::string> { "1", "2", "3" }));
}

TEST(TyrTests, TyrCommonFormatterHandlesNullableWrappers)
{
    EXPECT_EQ(fmt::format("{}", std::optional<int> {}), "<nullopt>");
    EXPECT_EQ(fmt::format("{}", std::optional<int> { 7 }), "7");
    EXPECT_EQ(fmt::format("{}", std::shared_ptr<int> {}), "<nullptr>");
    EXPECT_EQ(fmt::format("{}", std::make_shared<int>(9)), "9");
}

TEST(TyrTests, TyrCommonAssociativeContainerFormatterFormatsFlatHashMap)
{
    auto values = UnorderedMap<int, std::string_view> {};
    values.emplace(1, "one");

    EXPECT_EQ(fmt::format("{}", values), "{1: one}");
}

}
