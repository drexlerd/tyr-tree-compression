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

#include "tyr/datalog/lifted/workspaces/program.hpp"

#include "tyr/datalog/lifted/policies/aggregation.hpp"
#include "tyr/datalog/lifted/policies/annotation.hpp"
#include "tyr/datalog/lifted/policies/termination.hpp"

namespace tyr::datalog
{
template<OrAnnotationPolicyConcept<LiftedTag> OrAP,
         AndAnnotationPolicyConcept<LiftedTag> AndAP,
         TerminationPolicyConcept<LiftedTag> TP,
         RuleCostPolicyConcept<LiftedTag> CP>
ProgramWorkspace<LiftedTag, OrAP, AndAP, TP, CP>::ProgramWorkspace(ProgramContext& context,
                                                                   const ConstProgramWorkspace<LiftedTag>& cws,
                                                                   OrAP or_ap,
                                                                   AndAP and_ap,
                                                                   TP tp,
                                                                   CP cost_policy) :
    program_repository(context.get_program_repository()),
    workspace_repository(context.get_workspace_repository()),
    facts(context.get_program().get_predicates<::tyr::formalism::FluentTag>(),
          context.get_program().get_functions<::tyr::formalism::FluentTag>(),
          context.get_domains().fluent_predicate_domains,
          context.get_domains().fluent_function_domains,
          context.get_program().get_objects().size(),
          context.get_program().get_atoms<::tyr::formalism::FluentTag>(),
          context.get_program().get_fterm_values<::tyr::formalism::FluentTag>(),
          context.get_workspace_repository()),
    or_ap(or_ap),
    and_annot(),
    numeric_and_annot(),
    numeric_support_selector(),
    tp(tp),
    cost_policy(std::move(cost_policy)),
    rules(),
    planning_builder(),
    datalog_builder(),
    schedulers(create_schedulers(context.get_strata(),
                                 context.get_listeners(),
                                 program_repository,
                                 context.get_program().get_predicates<::tyr::formalism::FluentTag>().size(),
                                 context.get_program().get_functions<::tyr::formalism::FluentTag>().size())),
    cost_buckets(),
    statistics()
{
    for (ygg::uint_t i = 0; i < context.get_program().get_rules().size(); ++i)
        rules.emplace_back(
            cws.rules[i].has_value() ?
                std::make_unique<RuleWorkspace<AndAP>>(context.get_repository_factory(), program_repository, workspace_repository, *cws.rules[i], and_ap) :
                nullptr);
}

template struct ProgramWorkspace<LiftedTag, NoOrAnnotationPolicy<LiftedTag>, NoAndAnnotationPolicy<LiftedTag>, NoTerminationPolicy<LiftedTag>>;
template struct ProgramWorkspace<LiftedTag, OrAnnotationPolicy<LiftedTag>, AndAnnotationPolicy<LiftedTag, SumAggregation>, NoTerminationPolicy<LiftedTag>>;
template struct ProgramWorkspace<LiftedTag,
                                 OrAnnotationPolicy<LiftedTag>,
                                 AndAnnotationPolicy<LiftedTag, SumAggregation>,
                                 TerminationPolicy<LiftedTag, SumAggregation>>;
template struct ProgramWorkspace<LiftedTag, OrAnnotationPolicy<LiftedTag>, AndAnnotationPolicy<LiftedTag, MaxAggregation>, NoTerminationPolicy<LiftedTag>>;
template struct ProgramWorkspace<LiftedTag,
                                 OrAnnotationPolicy<LiftedTag>,
                                 AndAnnotationPolicy<LiftedTag, MaxAggregation>,
                                 TerminationPolicy<LiftedTag, MaxAggregation>>;
template struct ProgramWorkspace<LiftedTag,
                                 NoOrAnnotationPolicy<LiftedTag>,
                                 NoAndAnnotationPolicy<LiftedTag>,
                                 NoTerminationPolicy<LiftedTag>,
                                 RuleCostOverridePolicy<LiftedTag>>;
template struct ProgramWorkspace<LiftedTag,
                                 OrAnnotationPolicy<LiftedTag>,
                                 AndAnnotationPolicy<LiftedTag, SumAggregation>,
                                 NoTerminationPolicy<LiftedTag>,
                                 RuleCostOverridePolicy<LiftedTag>>;
template struct ProgramWorkspace<LiftedTag,
                                 OrAnnotationPolicy<LiftedTag>,
                                 AndAnnotationPolicy<LiftedTag, SumAggregation>,
                                 TerminationPolicy<LiftedTag, SumAggregation>,
                                 RuleCostOverridePolicy<LiftedTag>>;
template struct ProgramWorkspace<LiftedTag,
                                 OrAnnotationPolicy<LiftedTag>,
                                 AndAnnotationPolicy<LiftedTag, MaxAggregation>,
                                 NoTerminationPolicy<LiftedTag>,
                                 RuleCostOverridePolicy<LiftedTag>>;
template struct ProgramWorkspace<LiftedTag,
                                 OrAnnotationPolicy<LiftedTag>,
                                 AndAnnotationPolicy<LiftedTag, MaxAggregation>,
                                 TerminationPolicy<LiftedTag, MaxAggregation>,
                                 RuleCostOverridePolicy<LiftedTag>>;
template struct ProgramWorkspace<LiftedTag,
                                 OrAnnotationPolicy<LiftedTag>,
                                 AchieverAndAnnotationPolicy<LiftedTag, MaxAggregation>,
                                 TerminationPolicy<LiftedTag, MaxAggregation>,
                                 RuleCostOverridePolicy<LiftedTag>>;

ConstProgramWorkspace<LiftedTag>::ConstProgramWorkspace(ProgramContext& context) :
    facts(context.get_program().get_predicates<::tyr::formalism::StaticTag>(),
          context.get_program().get_functions<::tyr::formalism::StaticTag>(),
          context.get_domains().static_predicate_domains,
          context.get_domains().static_function_domains,
          context.get_program().get_objects().size(),
          context.get_program().get_atoms<::tyr::formalism::StaticTag>(),
          context.get_program().get_fterm_values<::tyr::formalism::StaticTag>(),
          context.get_program_repository()),
    rules()
{
    rules.resize(context.get_program().get_rules().size());
    for (ygg::uint_t i = 0; i < context.get_program().get_rules().size(); ++i)
    {
        const auto rule = context.get_program().get_rules()[i];
        rules[i].emplace(rule,
                         context.get_workspace_repository(),
                         context.get_domains().rule_domains.at(rule.get_index()).payload,
                         context.get_program().get_objects().size(),
                         context.get_program().get_predicates<::tyr::formalism::FluentTag>().size(),
                         facts.assignment_sets);
    }
}

}
