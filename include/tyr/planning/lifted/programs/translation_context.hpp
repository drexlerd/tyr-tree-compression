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

#ifndef TYR_PLANNING_LIFTED_PROGRAMS_TRANSLATION_CONTEXT_HPP_
#define TYR_PLANNING_LIFTED_PROGRAMS_TRANSLATION_CONTEXT_HPP_

#include "tyr/formalism/datalog/declarations.hpp"
#include "tyr/formalism/datalog/repository.hpp"
#include "tyr/formalism/planning/declarations.hpp"
#include "tyr/formalism/planning/repository.hpp"
#include "tyr/planning/programs/translation_context.hpp"

#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/semantics/equal_to.hpp>
#include <yggdrasil/semantics/hash.hpp>

namespace tyr::planning
{

template<>
struct D2PTranslationContext<LiftedTag>
{
    using StaticToStaticPredicateMapping = ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<::tyr::formalism::StaticTag>,
                                                             ::tyr::formalism::planning::PredicateView<::tyr::formalism::StaticTag>>;
    using FluentToFluentPredicateMapping = ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<::tyr::formalism::FluentTag>,
                                                             ::tyr::formalism::planning::PredicateView<::tyr::formalism::FluentTag>>;
    using FluentToDerivedPredicateMapping = ygg::UnorderedMap<::tyr::formalism::datalog::PredicateView<::tyr::formalism::FluentTag>,
                                                              ::tyr::formalism::planning::PredicateView<::tyr::formalism::DerivedTag>>;

    StaticToStaticPredicateMapping static_to_static_predicate;
    FluentToFluentPredicateMapping fluent_to_fluent_predicate;
    FluentToDerivedPredicateMapping fluent_to_derived_predicate;
};

template<>
struct P2DTranslationContext<LiftedTag>
{
    using StaticToStaticPredicateMapping = ygg::UnorderedMap<::tyr::formalism::planning::PredicateView<::tyr::formalism::StaticTag>,
                                                             ::tyr::formalism::datalog::PredicateView<::tyr::formalism::StaticTag>>;
    using FluentToFluentPredicateMapping = ygg::UnorderedMap<::tyr::formalism::planning::PredicateView<::tyr::formalism::FluentTag>,
                                                             ::tyr::formalism::datalog::PredicateView<::tyr::formalism::FluentTag>>;
    using DerivedToFluentPredicateMapping = ygg::UnorderedMap<::tyr::formalism::planning::PredicateView<::tyr::formalism::DerivedTag>,
                                                              ::tyr::formalism::datalog::PredicateView<::tyr::formalism::FluentTag>>;

    StaticToStaticPredicateMapping static_to_static_predicate;
    FluentToFluentPredicateMapping fluent_to_fluent_predicate;
    DerivedToFluentPredicateMapping derived_to_fluent_predicate;
};

}

#endif
