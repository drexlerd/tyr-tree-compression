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

#include <boost/json.hpp>
#include <filesystem>
#include <gtest/gtest.h>
#include <map>
#include <string>
#include <yggdrasil/serialization/json.hpp>
#include <yggdrasil/serialization/json_suite.hpp>

namespace tyr::tests
{
namespace
{
using ConfigValues = std::map<std::string, double>;
using FixtureValues = std::map<std::string, ConfigValues>;

std::string make_key(const boost::json::object& object)
{
    return ygg::common::as_string(object, "name", "case") + "|" + ygg::common::as_string(object, "domain_file", "case") + "|"
           + ygg::common::as_string(object, "task_file", "case");
}

FixtureValues load_fixture_values(const std::filesystem::path& fixture)
{
    const auto suite = ygg::common::load_json_file(ygg::common::root_path() / fixture);
    const auto& suite_object = ygg::common::as_object(suite, "suite");
    auto result = FixtureValues {};

    for (const auto& case_value : ygg::common::as_array(suite_object, "cases", "suite"))
    {
        const auto& object = ygg::common::as_object(case_value, "case");
        const auto* configs = object.if_contains("configs");
        if (!configs)
            continue;

        auto& values = result[make_key(object)];
        for (const auto& [mode, config] : configs->as_object())
        {
            const auto* h = config.as_object().if_contains("h");
            if (h)
                values.emplace(mode, boost::json::value_to<double>(*h));
        }
    }

    return result;
}

void check_kind(const std::string& kind)
{
    const auto root = std::filesystem::path("tests/unit/planning/heuristics") / kind;
    const auto hmax = load_fixture_values(root / "rpg_max.json");
    const auto hlmcut = load_fixture_values(root / "lmcut.json");
    const auto hstar = load_fixture_values(root / "hstar.json");

    for (const auto& [case_key, hstar_configs] : hstar)
    {
        const auto hmax_it = hmax.find(case_key);
        const auto hlmcut_it = hlmcut.find(case_key);
        if (hmax_it == hmax.end() || hlmcut_it == hlmcut.end())
            continue;

        for (const auto& [mode, hstar_value] : hstar_configs)
        {
            const auto hmax_config_it = hmax_it->second.find(mode);
            const auto hlmcut_config_it = hlmcut_it->second.find(mode);
            if (hmax_config_it == hmax_it->second.end() || hlmcut_config_it == hlmcut_it->second.end())
                continue;

            SCOPED_TRACE(kind + ":" + mode + ":" + case_key);
            EXPECT_LE(hmax_config_it->second, hlmcut_config_it->second);
            EXPECT_LE(hlmcut_config_it->second, hstar_value);
        }
    }
}
}

TEST(TyrPlanningHeuristicsCrossRPGTest, HMaxLeLMCutLeHStarWhereHStarIsKnown)
{
    check_kind("ground");
    check_kind("lifted");
}

}
