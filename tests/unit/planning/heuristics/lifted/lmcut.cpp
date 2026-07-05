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
#include <memory>
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
struct LMCutCase
{
    struct Expectations
    {
        std::optional<ygg::float_t> hmax;
        std::optional<ygg::float_t> hlmcut;
        std::optional<ygg::float_t> hstar;
    };

    std::string name;
    std::filesystem::path domain_file;
    std::filesystem::path task_file;
    Expectations unit;
    Expectations general;
};

void PrintTo(const LMCutCase& test_case, std::ostream* os) { *os << test_case.name << " (" << test_case.task_file << ")"; }

std::optional<ygg::float_t> parse_optional_float(const boost::json::object& object, const char* key)
{
    const auto* value = object.if_contains(key);
    if (!value)
        return std::nullopt;
    return boost::json::value_to<ygg::float_t>(*value);
}

LMCutCase::Expectations parse_expectations(const boost::json::object& object, const std::string& suffix)
{
    return LMCutCase::Expectations { parse_optional_float(object, ("hmax_" + suffix).c_str()),
                                     parse_optional_float(object, ("hlmcut_" + suffix).c_str()),
                                     parse_optional_float(object, ("hstar_" + suffix).c_str()) };
}

LMCutCase parse_case(const boost::json::object& suite, const boost::json::object& object)
{
    return LMCutCase { ygg::common::as_string(object, "name", "case"),
                       ygg::common::suite_path(suite, ygg::common::as_string(object, "domain_file", "case")),
                       ygg::common::suite_path(suite, ygg::common::as_string(object, "task_file", "case")),
                       parse_expectations(object, "unit"),
                       parse_expectations(object, "general") };
}

std::vector<LMCutCase> load_cases()
{
    const auto suite = ygg::common::load_json_file(ygg::common::root_path() / "tests/unit/planning/heuristics/lifted/lmcut.json");
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    auto result = std::vector<LMCutCase> {};
    for (const auto& case_value : ygg::common::as_array(suite_object, "cases", "suite"))
        result.push_back(parse_case(suite_object, ygg::common::as_object(case_value, "case")));
    return result;
}
}

class LMCutTest : public ::testing::TestWithParam<LMCutCase>
{
};

void check_initial_state(const LMCutCase::Expectations& expectations,
                         p::CostMode cost_mode,
                         p::TaskPtr<p::LiftedTag> task,
                         const p::StateView<p::LiftedTag>& state,
                         const std::shared_ptr<ygg::ExecutionContext>& execution_context)
{
    auto hmax = p::MaxRPGHeuristic<p::LiftedTag>::create(task, execution_context, cost_mode);
    auto lmcut = p::LMCutHeuristic<p::LiftedTag>::create(task, execution_context, cost_mode);

    const auto hmax_value = hmax->evaluate(state);
    const auto lmcut_value = lmcut->evaluate(state);

    ASSERT_NE(hmax_value, std::numeric_limits<ygg::float_t>::infinity());
    ASSERT_NE(lmcut_value, std::numeric_limits<ygg::float_t>::infinity());
    EXPECT_GE(lmcut_value, hmax_value);

    if (expectations.hmax)
    {
        EXPECT_EQ(hmax_value, *expectations.hmax);
    }

    if (expectations.hlmcut)
    {
        EXPECT_EQ(lmcut_value, *expectations.hlmcut);
    }

    if (expectations.hstar)
    {
        EXPECT_LE(lmcut_value, *expectations.hstar);
    }
}

bool should_check(const LMCutCase::Expectations& expectations) { return expectations.hmax || expectations.hlmcut || expectations.hstar; }

TEST_P(LMCutTest, InitialStateIsFiniteAndDominatesHMax)
{
    const auto& test_case = GetParam();
    auto task = p::Task<p::LiftedTag>::create(fp::Parser(test_case.domain_file).parse_task(test_case.task_file));
    auto execution_context = ygg::ExecutionContext::create(1);
    auto axiom_evaluator = p::AxiomEvaluatorFactory<p::LiftedTag>().create(task, execution_context);
    auto state_repository = p::StateRepositoryFactory<p::LiftedTag>().create(task, axiom_evaluator);
    auto successor_generator = p::SuccessorGeneratorFactory<p::LiftedTag>().create(task, execution_context, state_repository);
    const auto initial_node = successor_generator->get_initial_node();

    check_initial_state(test_case.unit, p::CostMode::UNIT, task, initial_node.get_state(), execution_context);

    if (should_check(test_case.general))
        check_initial_state(test_case.general, p::CostMode::GENERAL, task, initial_node.get_state(), execution_context);
}

INSTANTIATE_TEST_SUITE_P(TyrPlanningLMCut,
                         LMCutTest,
                         ::testing::ValuesIn(load_cases()),
                         [](const testing::TestParamInfo<LMCutCase>& info) { return info.param.name; });

}
