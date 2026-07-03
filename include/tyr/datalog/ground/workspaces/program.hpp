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

#ifndef TYR_DATALOG_GROUND_WORKSPACES_PROGRAM_HPP_
#define TYR_DATALOG_GROUND_WORKSPACES_PROGRAM_HPP_

#include "tyr/datalog/ground/policies/annotation.hpp"
#include "tyr/datalog/ground/policies/cost.hpp"
#include "tyr/datalog/ground/policies/termination.hpp"
#include "tyr/datalog/ground/workspaces/facts.hpp"
#include "tyr/datalog/ground/workspaces/rule.hpp"
#include "tyr/datalog/policies/annotation_concept.hpp"
#include "tyr/datalog/policies/cost_concept.hpp"
#include "tyr/datalog/policies/termination_concept.hpp"
#include "tyr/datalog/workspaces/program.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <type_traits>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/types.hpp>

namespace tyr::datalog
{

struct GroundQueueStatistics
{
    ygg::uint_t num_queue_pushes = 0;
    ygg::uint_t num_queue_pops = 0;
    ygg::uint_t num_stale_queue_pops = 0;
    ygg::uint_t num_rules_fired = 0;
    ygg::uint_t num_facts_derived = 0;
    ygg::uint_t max_queue_size = 0;
};

template<>
struct ConstProgramWorkspace<GroundTag>
{
    ::tyr::formalism::datalog::GroundProgramView program;

    ygg::UnorderedMap<::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>, std::vector<::tyr::formalism::datalog::GroundRuleView>>
        fluent_precondition_to_rules;

    explicit ConstProgramWorkspace(::tyr::formalism::datalog::GroundProgramView program);
};

template<>
struct ProgramWorkspace<GroundTag>
{
    template<OrAnnotationPolicyConcept<GroundTag> OrAP = NoOrAnnotationPolicy<GroundTag>,
             AndAnnotationPolicyConcept<GroundTag> AndAP = NoAndAnnotationPolicy<GroundTag>,
             TerminationPolicyConcept<GroundTag> TP = NoTerminationPolicy<GroundTag>,
             RuleCostPolicyConcept<GroundTag> CP = RuleCostPolicy<GroundTag>>
    struct Instance
    {
        FactsWorkspace<GroundTag> facts;
        OrAP or_ap;
        AndAP and_ap;
        GroundSelectedPredicateAnnotations and_annot;
        TP tp;
        CP cost_policy;
        RuleWorkspace<GroundTag> rules;
        GroundQueueStatistics statistics;

        explicit Instance(const ConstProgramWorkspace<GroundTag>& cws, OrAP or_ap_ = OrAP(), AndAP and_ap_ = AndAP(), TP tp_ = TP(), CP cost_policy_ = CP()) :
            facts(),
            or_ap(std::move(or_ap_)),
            and_ap(std::move(and_ap_)),
            and_annot(),
            tp(std::move(tp_)),
            cost_policy(std::move(cost_policy_)),
            rules(cws.program),
            statistics()
        {
            facts.fluent_atoms.reserve(cws.program.template get_atoms<::tyr::formalism::FluentTag>().size());
        }

        void clear_costs() { cost_policy.clear(); }
    };
};

}

#endif
