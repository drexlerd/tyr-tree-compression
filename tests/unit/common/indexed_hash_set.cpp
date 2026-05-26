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
#include <tyr/common/indexed_hash_set.hpp>

namespace tyr::tests
{

struct IndexedHashSetTestTag
{
};

}

namespace tyr
{

template<>
struct Index<tests::IndexedHashSetTestTag> : IndexMixin<Index<tests::IndexedHashSetTestTag>>
{
    using IndexMixin::IndexMixin;
};

template<>
struct Data<tests::IndexedHashSetTestTag>
{
    int value {};

    auto identifying_members() const noexcept { return std::tie(value); }
};

}

namespace tyr::tests
{

TEST(TyrTests, TyrCommonIndexedHashSetFindsAndContainsInsertedValues)
{
    auto set = IndexedHashSet<IndexedHashSetTestTag>();
    const auto first = Data<IndexedHashSetTestTag> { 1 };
    const auto second = Data<IndexedHashSetTestTag> { 2 };
    const auto missing = Data<IndexedHashSetTestTag> { 3 };

    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
    EXPECT_FALSE(set.contains(first));

    auto [first_index, inserted_first] = set.insert(first);
    EXPECT_TRUE(inserted_first);
    EXPECT_EQ(first_index.get_value(), uint_t { 0 });
    EXPECT_TRUE(set.contains(first));
    EXPECT_EQ(set.find(first), first_index);
    EXPECT_EQ(set[first_index].value, 1);

    auto [second_index, inserted_second] = set.insert(second);
    EXPECT_TRUE(inserted_second);
    EXPECT_EQ(second_index.get_value(), uint_t { 1 });
    EXPECT_TRUE(set.contains(second));

    auto [duplicate_index, inserted_duplicate] = set.insert(first);
    EXPECT_FALSE(inserted_duplicate);
    EXPECT_EQ(duplicate_index, first_index);
    EXPECT_EQ(set.size(), 2);
    EXPECT_FALSE(set.empty());
    EXPECT_FALSE(set.contains(missing));

    set.clear();
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
    EXPECT_FALSE(set.contains(first));
}

}
