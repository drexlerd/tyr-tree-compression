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

#ifndef TYR_DATALOG_GROUND_WORKSPACES_RULE_HPP_
#define TYR_DATALOG_GROUND_WORKSPACES_RULE_HPP_

#include "tyr/formalism/datalog/repository.hpp"

#include <tuple>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace tyr::datalog
{

struct GroundConstRuleWorkspace
{
    ::tyr::formalism::datalog::GroundRuleView rule;

    explicit GroundConstRuleWorkspace(::tyr::formalism::datalog::GroundRuleView rule_) : rule(rule_) {}
};

struct GroundQueueEntry
{
    ygg::uint_t unsatisfied_count;
    ::tyr::formalism::datalog::GroundRuleView rule;

    auto identifying_members() const noexcept { return std::make_tuple(unsatisfied_count, rule.get_index()); }
};

struct GroundRuleWorkspace
{
    std::vector<ygg::uint_t> unsatisfied_counts;
    std::vector<bool> fired_rules;
    std::vector<GroundQueueEntry> queue_storage;

    explicit GroundRuleWorkspace(::tyr::formalism::datalog::GroundProgramView program) { queue_storage.reserve(program.get_ground_rules().size()); }
};

}

#endif
