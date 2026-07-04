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

#include "tyr/formalism/formalism.hpp"
#include "tyr/planning/planning.hpp"

#include <boost/json.hpp>
#include <filesystem>
#include <gtest/gtest.h>
#include <limits>
#include <optional>
#include <ostream>
#include <string>
#include <vector>
#include <yggdrasil/serialization/json.hpp>
#include <yggdrasil/serialization/json_suite.hpp>

namespace fp = tyr::formalism::planning;
namespace p = tyr::planning;

namespace tyr::tests
{
namespace
{
struct GroundLMCutCase
{
    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
    std::optional<ygg::float_t> expected_hmax;
    std::optional<ygg::float_t> expected_lmcut;
    std::optional<ygg::float_t> h_star_unit_cost_one;
};

void PrintTo(const GroundLMCutCase& test_case, std::ostream* os) { *os << test_case.name << " (" << test_case.task_file << ")"; }

std::optional<ygg::float_t> parse_optional_float(const boost::json::object& object, const char* key)
{
    const auto* value = object.if_contains(key);
    if (!value)
        return std::nullopt;
    return boost::json::value_to<ygg::float_t>(*value);
}

GroundLMCutCase parse_case(const boost::json::object& suite, const boost::json::object& object)
{
    return GroundLMCutCase { ygg::common::as_string(object, "name", "case"),
                             ygg::common::suite_path(suite, ygg::common::as_string(object, "domain_file", "case")),
                             ygg::common::suite_path(suite, ygg::common::as_string(object, "task_file", "case")),
                             parse_optional_float(object, "expected_hmax"),
                             parse_optional_float(object, "expected_lmcut"),
                             parse_optional_float(object, "h_star_unit_cost_one") };
}

std::vector<GroundLMCutCase> load_cases()
{
    const auto suite = ygg::common::load_json_file(ygg::common::root_path() / "tests/unit/planning/heuristics/ground/lmcut.json");
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    auto result = std::vector<GroundLMCutCase> {};
    for (const auto& case_value : ygg::common::as_array(suite_object, "cases", "suite"))
        result.push_back(parse_case(suite_object, ygg::common::as_object(case_value, "case")));
    return result;
}
}

class GroundLMCutTest : public ::testing::TestWithParam<GroundLMCutCase>
{
};

TEST_P(GroundLMCutTest, InitialStateIsFiniteDominatesHMaxAndDoesNotExceedHStar)
{
    const auto& test_case = GetParam();
    auto lifted_task = p::Task<p::LiftedTag>(fp::Parser(test_case.domain_file).parse_task(test_case.task_file));
    auto grounding_context = ygg::ExecutionContext(1);
    auto ground_task = lifted_task.instantiate_ground_task(grounding_context).task;

    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::GroundTag>().create(ground_task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::GroundTag>().create(ground_task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::GroundTag>().create(ground_task, execution_context, state_repository);
    const auto initial_node = successor_generator->get_initial_node();

    auto hmax = p::MaxRPGHeuristic<p::GroundTag>::create(ground_task, execution_context, p::CostMode::UNIT);
    auto lmcut = p::LMCutHeuristic<p::GroundTag>::create(ground_task, execution_context, p::CostMode::UNIT);

    const auto hmax_value = hmax->evaluate(initial_node.get_state());
    const auto lmcut_value = lmcut->evaluate(initial_node.get_state());

    ASSERT_NE(hmax_value, std::numeric_limits<ygg::float_t>::infinity());
    ASSERT_NE(lmcut_value, std::numeric_limits<ygg::float_t>::infinity());
    EXPECT_GE(lmcut_value, hmax_value);

    if (test_case.expected_hmax)
    {
        EXPECT_EQ(hmax_value, *test_case.expected_hmax);
    }

    if (test_case.expected_lmcut)
    {
        EXPECT_EQ(lmcut_value, *test_case.expected_lmcut);
    }

    if (test_case.h_star_unit_cost_one)
    {
        EXPECT_LE(lmcut_value, *test_case.h_star_unit_cost_one);
    }
}

INSTANTIATE_TEST_SUITE_P(TyrPlanningGroundLMCut,
                         GroundLMCutTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<GroundLMCutCase>& info) { return info.param.name; });

}
