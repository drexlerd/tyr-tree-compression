#include "tyr/common/associative_containers.hpp"

#include "tyr/common/comparators.hpp"
#include "tyr/common/equal_to.hpp"
#include "tyr/common/hash.hpp"

#include <gtest/gtest.h>

#include <string>

namespace tyr::tests
{

TEST(TyrTests, TyrCommonAssociativeContainerAliasesUseCommonComparators)
{
    auto unordered_set = UnorderedSet<int> {};
    unordered_set.insert(3);
    EXPECT_TRUE(unordered_set.contains(3));

    auto unordered_map = UnorderedMap<int, std::string> {};
    unordered_map.emplace(1, "one");
    EXPECT_EQ(unordered_map.at(1), "one");

    auto set = Set<int> {};
    set.insert(2);
    set.insert(1);
    EXPECT_EQ(*set.begin(), 1);

    auto map = Map<int, std::string> {};
    map.emplace(2, "two");
    map.emplace(1, "one");
    EXPECT_EQ(map.begin()->first, 1);
}

}  // namespace tyr::tests
