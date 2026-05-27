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
#include <tyr/common/associative_containers.hpp>
#include <tyr/common/cista_comparators.hpp>
#include <tyr/common/block_array_comparators.hpp>
#include <tyr/common/dynamic_bitset_comparators.hpp>
#include <tyr/common/observer_ptr_comparators.hpp>
#include <tyr/common/raw_vector_comparators.hpp>
#include <tyr/common/segmented_vector_comparators.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <span>
#include <tuple>
#include <vector>

namespace tyr::tests
{

struct ComparatorContext
{
};

struct IdentifiableComparatorValue
{
    int first;
    int second;

    auto identifying_members() const noexcept { return std::tie(first, second); }
};

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

TEST(TyrTests, TyrCommonBitPackedArrayComparatorOrdersViews)
{
    auto lhs_storage = std::vector<uint8_t>(1, 0);
    auto rhs_storage = std::vector<uint8_t>(1, 0);

    using View = BasicBitPackedArrayView<uint8_t, bit::ForwardingBlockCoder<uint8_t>>;
    auto lhs = View(lhs_storage.data(), 2, 3, 0);
    auto rhs = View(rhs_storage.data(), 2, 3, 0);

    const auto lhs_values = std::array<uint8_t, 2> { 1, 2 };
    const auto rhs_values = std::array<uint8_t, 2> { 1, 3 };
    lhs = std::span<const uint8_t>(lhs_values);
    rhs = std::span<const uint8_t>(rhs_values);

    EXPECT_TRUE(Less<View> {}(lhs, rhs));
    EXPECT_FALSE(Less<View> {}(rhs, lhs));
}

TEST(TyrTests, TyrCommonDynamicBitsetComparatorOrdersBoostDynamicBitsets)
{
    auto lhs = boost::dynamic_bitset<>(8);
    auto rhs = boost::dynamic_bitset<>(8);

    lhs.set(1);
    rhs.set(2);

    EXPECT_TRUE(Less<boost::dynamic_bitset<>> {}(lhs, rhs));
    EXPECT_FALSE(Less<boost::dynamic_bitset<>> {}(rhs, lhs));
}

TEST(TyrTests, TyrCommonDynamicBitsetComparatorOrdersBitsetSpans)
{
    auto lhs_blocks = std::vector<uint64_t>(BitsetSpan<uint64_t>::num_blocks(8), 0);
    auto rhs_blocks = std::vector<uint64_t>(BitsetSpan<uint64_t>::num_blocks(8), 0);

    auto lhs = BitsetSpan<uint64_t>(lhs_blocks.data(), 8);
    auto rhs = BitsetSpan<uint64_t>(rhs_blocks.data(), 8);

    lhs.set(1);
    rhs.set(2);

    EXPECT_TRUE(Less<BitsetSpan<uint64_t>> {}(lhs, rhs));
    EXPECT_FALSE(Less<BitsetSpan<uint64_t>> {}(rhs, lhs));
}

TEST(TyrTests, TyrCommonObserverPtrComparatorOrdersPointees)
{
    const auto lhs_value = 1;
    const auto rhs_value = 2;

    const auto lhs = make_observer(lhs_value);
    const auto rhs = make_observer(rhs_value);

    EXPECT_TRUE(Less<ObserverPtr<const int>> {}(lhs, rhs));
    EXPECT_FALSE(Less<ObserverPtr<const int>> {}(rhs, lhs));
}

TEST(TyrTests, TyrCommonRawVectorComparatorOrdersViews)
{
    auto pool = RawVectorPool<uint8_t, int, 32>();
    const auto lhs_index = pool.insert(std::vector<int> { 1, 2 });
    const auto rhs_index = pool.insert(std::vector<int> { 1, 3 });

    const auto lhs = pool[lhs_index];
    const auto rhs = pool[rhs_index];

    EXPECT_TRUE((Less<RawVectorView<uint8_t, int>> {}(lhs, rhs)));
    EXPECT_FALSE((Less<RawVectorView<uint8_t, int>> {}(rhs, lhs)));
}

TEST(TyrTests, TyrCommonSegmentedVectorComparatorOrdersValues)
{
    auto lhs = SegmentedVector<int, 2>();
    lhs.push_back(1);
    lhs.push_back(2);

    auto rhs = SegmentedVector<int, 2>();
    rhs.push_back(1);
    rhs.push_back(3);

    EXPECT_TRUE((Less<SegmentedVector<int, 2>> {}(lhs, rhs)));
    EXPECT_FALSE((Less<SegmentedVector<int, 2>> {}(rhs, lhs)));
}

TEST(TyrTests, TyrCommonStlComparatorsOrderAssociativeContainers)
{
    const auto lhs_set = std::set<int> { 1, 2 };
    const auto rhs_set = std::set<int> { 1, 3 };
    EXPECT_TRUE(Less<std::set<int>> {}(lhs_set, rhs_set));

    const auto lhs_map = std::map<int, int> { { 1, 2 } };
    const auto rhs_map = std::map<int, int> { { 1, 3 } };
    EXPECT_TRUE((Less<std::map<int, int>> {}(lhs_map, rhs_map)));
}

TEST(TyrTests, TyrCommonTyrAssociativeContainerAliasesOrderValues)
{
    const auto lhs_set = Set<int> { 1, 2 };
    const auto rhs_set = Set<int> { 1, 3 };
    EXPECT_TRUE(Less<Set<int>> {}(lhs_set, rhs_set));
    EXPECT_FALSE(Less<Set<int>> {}(rhs_set, lhs_set));

    const auto lhs_map = Map<int, int> { { 1, 2 } };
    const auto rhs_map = Map<int, int> { { 1, 3 } };
    EXPECT_TRUE((Less<Map<int, int>> {}(lhs_map, rhs_map)));
    EXPECT_FALSE((Less<Map<int, int>> {}(rhs_map, lhs_map)));
}

TEST(TyrTests, TyrCommonCistaLessAdaptersOrderOffsetString)
{
    auto lhs = ::cista::offset::string {};
    lhs = "ab";

    auto rhs = ::cista::offset::string {};
    rhs = "ac";

    EXPECT_TRUE(Less<::cista::offset::string> {}(lhs, rhs));
    EXPECT_FALSE(Less<::cista::offset::string> {}(rhs, lhs));
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

TEST(TyrTests, TyrCommonCistaLessAdaptersOrderOffsetVectorViews)
{
    auto lhs = ::cista::offset::vector<int> {};
    lhs.emplace_back(1);
    lhs.emplace_back(2);

    auto rhs = ::cista::offset::vector<int> {};
    rhs.emplace_back(1);
    rhs.emplace_back(3);

    const auto context = ComparatorContext {};
    using Vector = ::cista::offset::vector<int>;
    using VectorView = View<Vector, ComparatorContext>;

    EXPECT_TRUE(Less<VectorView> {}(VectorView(lhs, context), VectorView(rhs, context)));
    EXPECT_FALSE(Less<VectorView> {}(VectorView(rhs, context), VectorView(lhs, context)));
}

TEST(TyrTests, TyrCommonCistaLessAdaptersOrderOptionalViews)
{
    auto lhs = ::cista::optional<int> {};
    lhs = 2;

    auto rhs = ::cista::optional<int> {};
    rhs = 3;

    const auto context = ComparatorContext {};
    using OptionalView = View<::cista::optional<int>, ComparatorContext>;

    EXPECT_TRUE(Less<OptionalView> {}(OptionalView(lhs, context), OptionalView(rhs, context)));
    EXPECT_FALSE(Less<OptionalView> {}(OptionalView(rhs, context), OptionalView(lhs, context)));
}

TEST(TyrTests, TyrCommonIdentifiableComparatorOrdersMembersAndTuples)
{
    const auto lhs = IdentifiableComparatorValue { 1, 2 };
    const auto rhs = IdentifiableComparatorValue { 1, 3 };
    const auto rhs_members = std::tie(rhs.first, rhs.second);

    EXPECT_TRUE(Less<IdentifiableComparatorValue> {}(lhs, rhs));
    EXPECT_TRUE(Less<IdentifiableComparatorValue> {}(lhs, rhs_members));
    EXPECT_FALSE(Less<IdentifiableComparatorValue> {}(rhs_members, lhs));
}

TEST(TyrTests, TyrCommonCistaLessAdaptersOrderVariantViews)
{
    using Variant = ::cista::offset::variant<int, double>;

    auto lhs = Variant {};
    lhs = 2;

    auto rhs = Variant {};
    rhs = 3;

    const auto context = ComparatorContext {};
    using VariantView = View<Variant, ComparatorContext>;

    EXPECT_TRUE(Less<Variant> {}(lhs, rhs));
    EXPECT_FALSE(Less<Variant> {}(rhs, lhs));
    EXPECT_TRUE(Less<VariantView> {}(VariantView(lhs, context), VariantView(rhs, context)));
    EXPECT_FALSE(Less<VariantView> {}(VariantView(rhs, context), VariantView(lhs, context)));
}

}
