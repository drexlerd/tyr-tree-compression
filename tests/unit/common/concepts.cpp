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
#include <tyr/common/concepts.hpp>
#include <tyr/common/comparators.hpp>
#include <tyr/common/equal_to.hpp>
#include <tyr/common/hash.hpp>

#include <cstddef>
#include <span>
#include <vector>

namespace tyr::tests
{

struct IdentifiableConceptFixture
{
    auto identifying_members() const noexcept { return 0; }
};

struct NonIdentifiableConceptFixture
{
};

struct InvalidHashFixture
{
    bool operator()(int) const { return true; }
};

struct ValidHashFixture
{
    std::size_t operator()(int) const { return 0; }
};

struct InvalidEqualToFixture
{
    int operator()(int, int) const { return 0; }
};

struct ValidEqualToFixture
{
    bool operator()(int, int) const { return true; }
};

struct InvalidLessFixture
{
    int operator()(int, int) const { return 0; }
};

struct ValidLessFixture
{
    bool operator()(int, int) const { return false; }
};

TEST(TyrTests, TyrCommonConceptsHeaderExposesReusableConcepts)
{
    static_assert(Identifiable<IdentifiableConceptFixture>);
    static_assert(InputRangeOf<std::vector<int>, int>);
    static_assert(InputRangeOf<std::span<const int>, int>);
    static_assert(TriviallyCopyable<int>);
    static_assert(SameAsIgnoringCvref<const int&, int>);
    static_assert(UnsignedIntegralSameAsIgnoringConst<const unsigned int, unsigned int>);
    static_assert(!UnsignedIntegralSameAsIgnoringConst<unsigned int, unsigned long>);
    static_assert(!SameAsIgnoringCvref<const int&, double>);
    static_assert(!TriviallyCopyable<std::vector<int>>);
    static_assert(HashFor<Hash<int>, int>);
    static_assert(HashFor<ValidHashFixture, int>);
    static_assert(!HashFor<InvalidHashFixture, int>);
    static_assert(EqualToFor<EqualTo<int>, int>);
    static_assert(EqualToFor<ValidEqualToFixture, int>);
    static_assert(!EqualToFor<InvalidEqualToFixture, int>);
    static_assert(LessFor<Less<int>, int>);
    static_assert(LessFor<ValidLessFixture, int>);
    static_assert(!LessFor<InvalidLessFixture, int>);
    static_assert(Hashable<int>);
    static_assert(EqualityComparableByEqualTo<int>);
    static_assert(OrderedByLess<int>);
    static_assert(!Identifiable<NonIdentifiableConceptFixture>);

    SUCCEED();
}

}
