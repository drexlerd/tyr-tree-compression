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

#include <span>
#include <vector>

namespace tyr::tests
{

struct IdentifiableConceptFixture
{
    auto identifying_members() const noexcept { return 0; }
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

    SUCCEED();
}

}
