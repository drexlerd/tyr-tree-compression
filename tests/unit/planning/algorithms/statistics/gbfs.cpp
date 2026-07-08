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

#include "search_statistics.hpp"

namespace p = tyr::planning;

namespace tyr::tests
{
namespace
{
template<p::TaskKind Kind>
void check_statistics(const SearchStatistics& expected, const SearchCase& test_case, const std::string& heuristic_name)
{
    auto context = create_search_context<Kind>(test_case.domain_file, test_case.task_file);
    auto heuristic = create_search_heuristic<Kind>(heuristic_name, context);
    auto event_handler = p::gbfs_lazy::DefaultEventHandler<Kind>::create();

    auto options = p::gbfs_lazy::Options<Kind>();
    options.event_handler = event_handler;
    const auto result = p::gbfs_lazy::find_solution(*context.task, *context.successor_generator, *heuristic, options);

    expect_statistics(expected, event_handler->get_statistics(), result);
}
}

class GbfsStatisticsTest : public ::testing::TestWithParam<SearchCase>
{
};

TEST_P(GbfsStatisticsTest, StatisticsMatchFixture)
{
    const auto& test_case = GetParam();

    for (const auto& [key, expected] : test_case.configs)
    {
        SCOPED_TRACE(key);
        const auto [kind, heuristic_name] = parse_config_key(key);
        if (kind == "ground")
            check_statistics<p::GroundTag>(expected, test_case, heuristic_name);
        else
            check_statistics<p::LiftedTag>(expected, test_case, heuristic_name);
    }
}

INSTANTIATE_TEST_SUITE_P(TyrPlanningGbfsStatistics,
                         GbfsStatisticsTest,
                         ::testing::ValuesIn(load_search_cases("tests/unit/planning/algorithms/statistics/gbfs.json")),
                         [](const testing::TestParamInfo<SearchCase>& info) { return info.param.name; });

}
