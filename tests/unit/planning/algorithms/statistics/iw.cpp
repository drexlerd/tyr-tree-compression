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
struct IwExpectation
{
    std::optional<p::SearchStatus> expected_status;
    std::optional<ygg::uint_t> expected_plan_length;
    std::optional<ygg::uint_t> expected_solution_arity;
    std::optional<p::ProgressStatistics::Snapshot> counters;
};

struct IwCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
    ygg::uint_t max_arity;
    std::vector<std::pair<std::string, IwExpectation>> configs;
};

void PrintTo(const IwCase& test_case, std::ostream* os) { *os << test_case.name << " (" << test_case.task_file << ")"; }

IwExpectation parse_expectation(const boost::json::object& object)
{
    auto result = IwExpectation {};
    if (const auto value = ygg::common::find_string(object, "expected_status", "case"))
        result.expected_status = parse_search_status(*value);
    if (const auto value = ygg::common::find_uint_t(object, "expected_plan_length", "case"))
        result.expected_plan_length = *value;
    if (const auto value = ygg::common::find_uint_t(object, "expected_solution_arity", "case"))
        result.expected_solution_arity = *value;
    result.counters = parse_optional_counters(object);
    return result;
}

IwCase parse_case(const boost::json::object& suite, const boost::json::object& object)
{
    auto result = IwCase { ygg::common::as_string(object, "name", "case"),
                           ygg::common::suite_path(suite, ygg::common::as_string(object, "domain_file", "case")),
                           ygg::common::suite_path(suite, ygg::common::as_string(object, "task_file", "case")),
                           ygg::common::as_uint_t(suite, "max_arity", "suite"),
                           {} };
    for (const auto kind : { "ground", "lifted" })
        if (const auto* value = object.if_contains(kind))
            result.configs.emplace_back(kind, parse_expectation(value->as_object()));
    return result;
}

std::vector<IwCase> load_cases()
{
    const auto suite = ygg::common::load_json_file(ygg::common::root_path() / "tests/unit/planning/algorithms/statistics/iw.json");
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    auto result = std::vector<IwCase> {};
    for (const auto& case_value : ygg::common::as_array(suite_object, "cases", "suite"))
        result.push_back(parse_case(suite_object, ygg::common::as_object(case_value, "case")));
    return result;
}

template<p::TaskKind Kind>
void check_expectation(const IwExpectation& expectation, const IwCase& test_case)
{
    auto context = create_search_context<Kind>(test_case.domain_file, test_case.task_file);
    auto brfs_solver = p::brfs::Solver<Kind> { context.task, context.successor_generator, p::brfs::Options<Kind> {} };
    brfs_solver.options.event_handler = p::brfs::DefaultEventHandler<Kind>::create();

    auto options = p::iw::Options<Kind> {};
    const auto event_handler = p::iw::DefaultEventHandler<Kind>::create();
    options.event_handler = event_handler;

    const auto result = p::iw::find_solution(brfs_solver, test_case.max_arity, options);

    const auto solution_arity = event_handler->get_statistics().get_solution_arity();
    if (result.status == p::SearchStatus::SOLVED)
    {
        EXPECT_TRUE(solution_arity);
        if (solution_arity)
        {
            EXPECT_LE(*solution_arity, test_case.max_arity);
        }
    }
    else
    {
        EXPECT_FALSE(solution_arity);
    }

    if (expectation.expected_status)
    {
        EXPECT_EQ(result.status, *expectation.expected_status);
    }
    if (expectation.expected_plan_length)
    {
        ASSERT_TRUE(result.plan);
        EXPECT_EQ(result.plan->get_length(), *expectation.expected_plan_length);
        EXPECT_TRUE(solution_arity);
    }
    if (expectation.expected_solution_arity)
    {
        ASSERT_TRUE(solution_arity);
        EXPECT_EQ(*solution_arity, *expectation.expected_solution_arity);
    }
    if (expectation.counters)
    {
        expect_counters(*expectation.counters, event_handler->get_search_statistics());
    }
}
}

class IwStatisticsTest : public ::testing::TestWithParam<IwCase>
{
};

TEST_P(IwStatisticsTest, MatchesExpectedOutcome)
{
    const auto& test_case = GetParam();

    for (const auto& [kind, expectation] : test_case.configs)
    {
        SCOPED_TRACE(kind);
        if (kind == "ground")
            check_expectation<p::GroundTag>(expectation, test_case);
        else
            check_expectation<p::LiftedTag>(expectation, test_case);
    }
}

INSTANTIATE_TEST_SUITE_P(TyrPlanningIwStatistics,
                         IwStatisticsTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<IwCase>& info) { return info.param.name; });

}
