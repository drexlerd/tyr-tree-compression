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

#include "../statistics.hpp"

#include <unordered_map>

namespace tyr::tests
{
namespace
{
struct BrfsAstarCase
{
    std::string name;
    SearchStatistics brfs;
    SearchStatistics astar;
};

std::unordered_map<std::string, SearchCase> index_cases(std::vector<SearchCase> cases)
{
    auto result = std::unordered_map<std::string, SearchCase> {};
    for (auto& test_case : cases)
        result.emplace(test_case.name, std::move(test_case));
    return result;
}

const SearchStatistics* find_config(const SearchCase& test_case, const std::string& key)
{
    for (const auto& [name, stats] : test_case.configs)
        if (name == key)
            return &stats;
    return nullptr;
}

std::vector<BrfsAstarCase> load_cases(const std::string& prefix, const std::filesystem::path& brfs_fixture, const std::filesystem::path& astar_fixture)
{
    auto astar_cases = index_cases(load_search_cases(astar_fixture));
    auto result = std::vector<BrfsAstarCase> {};

    for (const auto& brfs_case : load_search_cases(brfs_fixture))
    {
        const auto* brfs = find_config(brfs_case, "default");
        if (!brfs)
            continue;

        const auto astar_it = astar_cases.find(brfs_case.name);
        if (astar_it == astar_cases.end())
            continue;

        const auto* astar = find_config(astar_it->second, "blind_unit");
        if (!astar)
            continue;

        result.push_back(BrfsAstarCase { prefix + brfs_case.name, *brfs, *astar });
    }

    return result;
}

std::vector<BrfsAstarCase> load_all_cases()
{
    auto result = load_cases("Ground", "tests/unit/planning/algorithms/statistics/ground/brfs.json",
                             "tests/unit/planning/algorithms/statistics/ground/astar.json");
    auto lifted = load_cases("Lifted", "tests/unit/planning/algorithms/statistics/lifted/brfs.json",
                             "tests/unit/planning/algorithms/statistics/lifted/astar.json");
    result.insert(result.end(), lifted.begin(), lifted.end());
    return result;
}

void PrintTo(const BrfsAstarCase& test_case, std::ostream* os) { *os << test_case.name; }
}

class BrfsVsAstarStatisticsTest : public ::testing::TestWithParam<BrfsAstarCase>
{
};

TEST_P(BrfsVsAstarStatisticsTest, UnitCostPlanLengthMatches)
{
    const auto& test_case = GetParam();

    ASSERT_EQ(test_case.brfs.plan.has_value(), test_case.astar.plan.has_value());
    if (test_case.brfs.plan)
    {
        EXPECT_EQ(test_case.brfs.plan->length, test_case.astar.plan->length);
    }
}

INSTANTIATE_TEST_SUITE_P(TyrPlanningStatisticsCross,
                         BrfsVsAstarStatisticsTest,
                         ::testing::ValuesIn(load_all_cases()),
                         [](const testing::TestParamInfo<BrfsAstarCase>& info) { return info.param.name; });

}
