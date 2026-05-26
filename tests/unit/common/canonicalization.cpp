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
#include <tyr/common/canonicalization.hpp>
#include <tyr/common/index_mixins.hpp>

#include <tuple>

namespace tyr
{
struct CanonicalizationTestTag;

template<>
struct Index<CanonicalizationTestTag> : IndexMixin<Index<CanonicalizationTestTag>>
{
    using IndexMixin<Index<CanonicalizationTestTag>>::IndexMixin;
};

template<>
struct Data<CanonicalizationTestTag>
{
    uint_t value {};

    auto identifying_members() const noexcept { return std::tie(value); }
};
}

namespace tyr::tests
{
namespace
{
using Tag = CanonicalizationTestTag;
}

TEST(TyrTests, TyrCommonCanonicalizeIndexListSortsAndDeduplicates)
{
    auto list = IndexList<Tag> {};
    list.push_back(Index<Tag>(2));
    list.push_back(Index<Tag>(1));
    list.push_back(Index<Tag>(2));
    list.push_back(Index<Tag>(3));

    EXPECT_FALSE(is_canonical(list));

    canonicalize(list);

    ASSERT_EQ(list.size(), 3);
    EXPECT_TRUE(is_canonical(list));
    EXPECT_EQ(list[0].get_value(), 1);
    EXPECT_EQ(list[1].get_value(), 2);
    EXPECT_EQ(list[2].get_value(), 3);
}

TEST(TyrTests, TyrCommonCanonicalizeDataListSortsAndDeduplicates)
{
    auto list = DataList<Tag> {};
    list.push_back(Data<Tag> { .value = 3 });
    list.push_back(Data<Tag> { .value = 1 });
    list.push_back(Data<Tag> { .value = 3 });
    list.push_back(Data<Tag> { .value = 2 });

    EXPECT_FALSE(is_canonical(list));

    canonicalize(list);

    ASSERT_EQ(list.size(), 3);
    EXPECT_TRUE(is_canonical(list));
    EXPECT_EQ(list[0].value, 1);
    EXPECT_EQ(list[1].value, 2);
    EXPECT_EQ(list[2].value, 3);
}

TEST(TyrTests, TyrCommonCanonicalizeOptionalIsNoOp)
{
    auto value = ::cista::optional<int> { 4 };

    EXPECT_TRUE(is_canonical(value));
    canonicalize(value);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, 4);
}

}
