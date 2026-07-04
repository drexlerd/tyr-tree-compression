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

#ifndef TYR_DATALOG_GROUND_WORKSPACES_FACTS_HPP_
#define TYR_DATALOG_GROUND_WORKSPACES_FACTS_HPP_

#include "tyr/datalog/fact_sets.hpp"
#include "tyr/datalog/workspaces/facts.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>

namespace tyr::datalog
{

template<>
struct FactsWorkspace<GroundTag>
{
    TaggedFactSets<::tyr::formalism::StaticTag> static_fact_sets;
    TaggedFactSets<::tyr::formalism::FluentTag> fluent_fact_sets;
    ygg::UnorderedSet<::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>> fluent_atoms;
    ygg::UnorderedMap<::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::StaticTag>, ygg::ClosedInterval<ygg::float_t>> static_fterm_intervals;
    ygg::UnorderedMap<::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>, ygg::ClosedInterval<ygg::float_t>> fluent_fterm_intervals;

    explicit FactsWorkspace(::tyr::formalism::datalog::ProgramView<GroundTag> program) :
        static_fact_sets(program.template get_predicates<::tyr::formalism::StaticTag>(),
                         program.template get_functions<::tyr::formalism::StaticTag>(),
                         program.template get_atoms<::tyr::formalism::StaticTag>(),
                         program.template get_fterm_values<::tyr::formalism::StaticTag>(),
                         program.get_context()),
        fluent_fact_sets(program.template get_predicates<::tyr::formalism::FluentTag>(),
                         program.template get_functions<::tyr::formalism::FluentTag>(),
                         program.get_context()),
        fluent_atoms(),
        static_fterm_intervals(),
        fluent_fterm_intervals()
    {
    }
};

}

#endif
