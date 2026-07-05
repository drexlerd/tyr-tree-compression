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

#include "planning/parser.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <tyr/formalism/formalism.hpp>
#include <tyr/planning/planning.hpp>
#include <yggdrasil/serialization/json.hpp>
#include <yggdrasil/serialization/json_suite.hpp>

namespace p = tyr::planning;
namespace f = tyr::formalism;
namespace fp = tyr::formalism::planning;

namespace tyr::tests
{
namespace
{
/// @brief Compare statistics that must be the same for both configurations
/// Notes:
/// - plan length may differ because two plans of different length may have same optimal cost.
/// - number of generated states may differ because of f value improvements due to differences in labeled successor node orderings.
struct SearchSummary
{
    std::optional<uint64_t> expanded_last_snapshot;
    p::SearchStatus status;
    ygg::float_t plan_cost;

    bool operator==(const SearchSummary& other) const { return status == other.status && f::apply(f::Eq {}, plan_cost, other.plan_cost); }
};

struct GroundVsLiftedCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
};

GroundVsLiftedCase parse_case(const boost::json::object& suite, const boost::json::object& object)
{
    return GroundVsLiftedCase { ygg::common::as_string(object, "name", "case"),
                                ygg::common::suite_path(suite, ygg::common::as_string(object, "domain_file", "case")),
                                ygg::common::suite_path(suite, ygg::common::as_string(object, "task_file", "case")) };
}

std::vector<GroundVsLiftedCase> load_cases()
{
    const auto suite = ygg::common::load_json_file(ygg::common::root_path() / "tests/unit/planning/algorithms/ground_vs_lifted.json");
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    auto result = std::vector<GroundVsLiftedCase> {};
    for (const auto& case_value : ygg::common::as_array(suite_object, "cases", "suite"))
        result.push_back(parse_case(suite_object, ygg::common::as_object(case_value, "case")));
    return result;
}

template<p::TaskKind Kind>
SearchSummary run_blind_astar(const fs::path& domain_filepath, const fs::path& problem_filepath)
{
    using TaskPtr = std::shared_ptr<p::Task<Kind>>;

    auto execution_context = ygg::ExecutionContext::create(1);

    TaskPtr task;
    if constexpr (std::same_as<Kind, p::GroundTag>)
        task = p::Task<p::LiftedTag>(make_test_parser(domain_filepath).parse_task(problem_filepath)).instantiate_ground_task(*execution_context).task;
    else if constexpr (std::same_as<Kind, p::LiftedTag>)
        task = p::Task<p::LiftedTag>::create(make_test_parser(domain_filepath).parse_task(problem_filepath));
    else
        static_assert(ygg::dependent_false<Kind>::value, "Missing case");

    auto axiom_evaluator = p::AxiomEvaluatorFactory<Kind>().create(task, execution_context);
    auto state_repository = p::StateRepositoryFactory<Kind>().create(task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<Kind>().create(task, execution_context, state_repository);
    auto heuristic = p::BlindHeuristic<Kind>::create();
    auto event_handler = p::astar_eager::DefaultEventHandler<Kind>::create(0);

    auto options = p::astar_eager::Options<Kind>();
    options.event_handler = event_handler;
    const auto result = p::astar_eager::find_solution(*task, *successor_generator, *heuristic, options);

    SearchSummary summary {};

    if (!event_handler->get_progress_statistics().get_snapshots().empty())
    {
        const auto& last = event_handler->get_progress_statistics().get_snapshots().back();
        summary.expanded_last_snapshot = last.get_num_expanded();
    }

    summary.status = result.status;
    if (result.plan.has_value())
    {
        summary.plan_cost = result.plan->get_cost();
    }

    return summary;
}
}

class GroundVsLiftedTest : public ::testing::TestWithParam<GroundVsLiftedCase>
{
};

TEST_P(GroundVsLiftedTest, BlindAStarMatches)
{
    const auto& param = GetParam();

    const auto lifted = run_blind_astar<p::LiftedTag>(param.domain_file, param.task_file);
    const auto grounded = run_blind_astar<p::GroundTag>(param.domain_file, param.task_file);

    EXPECT_EQ(lifted, grounded);
}

INSTANTIATE_TEST_SUITE_P(TyrTests,
                         GroundVsLiftedTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<GroundVsLiftedCase>& info) { return info.param.name; });
}
