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

#ifndef TYR_PLANNING_GROUND_HEURISTICS_RPG_HPP_
#define TYR_PLANNING_GROUND_HEURISTICS_RPG_HPP_

#include "tyr/datalog/ground/contexts/program.hpp"
#include "tyr/datalog/ground/policies/cost.hpp"
#include "tyr/datalog/ground/queue.hpp"
#include "tyr/datalog/ground/workspaces/program.hpp"
#include "tyr/datalog/policies/annotation_concept.hpp"
#include "tyr/datalog/policies/cost_concept.hpp"
#include "tyr/datalog/policies/termination_concept.hpp"
#include "tyr/formalism/datalog/builder.hpp"
#include "tyr/formalism/datalog/canonicalization.hpp"
#include "tyr/formalism/planning/merge_datalog.hpp"
#include "tyr/planning/applicability.hpp"
#include "tyr/planning/declarations.hpp"
#include "tyr/planning/ground/state_view.hpp"
#include "tyr/planning/ground_task.hpp"
#include "tyr/planning/heuristic.hpp"
#include "tyr/planning/heuristics/rpg.hpp"

#include <limits>

namespace tyr::planning
{

template<typename Derived,
         datalog::OrAnnotationPolicyConcept<GroundTag> OrAP,
         datalog::AndAnnotationPolicyConcept<GroundTag> AndAP,
         datalog::TerminationPolicyConcept<GroundTag> TP,
         datalog::RuleCostPolicyConcept<GroundTag> CP>
class RPGBase<GroundTag, Derived, OrAP, AndAP, TP, CP> : public Heuristic<GroundTag>
{
private:
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    RPGBase(TaskPtr<GroundTag> task, ygg::ExecutionContextPtr execution_context, const OrAP& or_ap, const AndAP& and_ap, const TP& tp);

    void set_goal(::tyr::formalism::planning::GroundConjunctiveConditionView goal) override;

    ygg::float_t evaluate(const StateView<GroundTag>& state) override;

    const auto& get_workspace() const noexcept { return m_workspace; }

protected:
    void set_action_binding_cost(::tyr::formalism::planning::ActionBindingView action_binding, datalog::Cost cost)
        requires datalog::MutableRuleCostPolicyConcept<CP, GroundTag>;

    datalog::Cost get_atom_cost(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> atom) const noexcept;

    datalog::Cost get_goal_cost() const noexcept;

protected:
    TaskPtr<GroundTag> m_task;
    ygg::ExecutionContextPtr m_execution_context;
    datalog::ProgramWorkspace<GroundTag>::Instance<OrAP, AndAP, TP, CP> m_workspace;
};

template<typename Derived,
         datalog::OrAnnotationPolicyConcept<GroundTag> OrAP,
         datalog::AndAnnotationPolicyConcept<GroundTag> AndAP,
         datalog::TerminationPolicyConcept<GroundTag> TP,
         datalog::RuleCostPolicyConcept<GroundTag> CP>
RPGBase<GroundTag, Derived, OrAP, AndAP, TP, CP>::RPGBase(TaskPtr<GroundTag> task,
                                                          ygg::ExecutionContextPtr execution_context,
                                                          const OrAP& or_ap,
                                                          const AndAP& and_ap,
                                                          const TP& tp) :
    m_task(std::move(task)),
    m_execution_context(std::move(execution_context)),
    m_workspace(m_task->get_rpg_program().get_datalog_program().get_const_program_workspace(), or_ap, and_ap, tp)
{
    m_workspace.tp.set_goals(m_task->get_rpg_program().get_goal());
}

template<typename Derived,
         datalog::OrAnnotationPolicyConcept<GroundTag> OrAP,
         datalog::AndAnnotationPolicyConcept<GroundTag> AndAP,
         datalog::TerminationPolicyConcept<GroundTag> TP,
         datalog::RuleCostPolicyConcept<GroundTag> CP>
void RPGBase<GroundTag, Derived, OrAP, AndAP, TP, CP>::set_goal(::tyr::formalism::planning::GroundConjunctiveConditionView goal)
{
    namespace fd = ::tyr::formalism::datalog;
    auto builder = fd::Builder();
    auto& repository = m_task->get_rpg_program().get_datalog_program().get_program_repository();
    auto condition_ptr = builder.get_builder<fd::GroundConjunctiveCondition>();
    auto& condition = *condition_ptr;
    condition.clear();

    const auto& p2d = m_task->get_rpg_program().get_translation_context().p2d.fluent_to_fluent_atom;
    for (const auto fact : goal.template get_facts<::tyr::formalism::PositiveTag>())
    {
        if (const auto atom = fact.get_atom())
        {
            auto literal_ptr = builder.get_builder<fd::GroundLiteral<::tyr::formalism::FluentTag>>();
            auto& literal = *literal_ptr;
            literal.clear();
            literal.atom = p2d.at(*atom).get_index();
            literal.polarity = true;
            fd::canonicalize(literal);
            condition.fluent_literals.push_back(repository.get_or_create(literal).first.get_index());
        }
    }

    fd::canonicalize(condition);
    m_workspace.tp.set_goals(repository.get_or_create(condition).first);
}

template<typename Derived,
         datalog::OrAnnotationPolicyConcept<GroundTag> OrAP,
         datalog::AndAnnotationPolicyConcept<GroundTag> AndAP,
         datalog::TerminationPolicyConcept<GroundTag> TP,
         datalog::RuleCostPolicyConcept<GroundTag> CP>
ygg::float_t RPGBase<GroundTag, Derived, OrAP, AndAP, TP, CP>::evaluate(const StateView<GroundTag>& state)
{
    m_workspace.facts.fluent_atoms.clear();
    const auto& p2d = m_task->get_rpg_program().get_translation_context().p2d.fluent_to_fluent_atom;
    for (const auto fact : state.get_fluent_facts_view())
        if (const auto atom = fact.get_atom())
            m_workspace.facts.fluent_atoms.insert(p2d.at(*atom));

    auto ctx = datalog::ProgramExecutionContext<GroundTag, OrAP, AndAP, TP, CP>(m_workspace,
                                                                                m_task->get_rpg_program().get_datalog_program().get_const_program_workspace());
    ctx.initialize();

    m_execution_context->arena().execute([&] { datalog::solve_ground_queue(ctx); });

    return m_workspace.tp.check(m_task->get_rpg_program().get_datalog_program().get_program(), m_workspace.facts) ?
               self().extract_cost_and_set_preferred_actions_impl(state) :
               std::numeric_limits<ygg::float_t>::infinity();
}

template<typename Derived,
         datalog::OrAnnotationPolicyConcept<GroundTag> OrAP,
         datalog::AndAnnotationPolicyConcept<GroundTag> AndAP,
         datalog::TerminationPolicyConcept<GroundTag> TP,
         datalog::RuleCostPolicyConcept<GroundTag> CP>
void RPGBase<GroundTag, Derived, OrAP, AndAP, TP, CP>::set_action_binding_cost(::tyr::formalism::planning::ActionBindingView action_binding, datalog::Cost cost)
    requires datalog::MutableRuleCostPolicyConcept<CP, GroundTag>
{
    if (const auto action = m_task->find_ground_action(action_binding))
    {
        const auto& mapping = m_task->get_rpg_program().get_ground_rule_to_action_mapping();
        for (const auto& [rule, mapped_action] : mapping)
            if (mapped_action.get_index() == action->get_index())
                m_workspace.cost_policy.set_cost(rule, cost);
    }
}

template<typename Derived,
         datalog::OrAnnotationPolicyConcept<GroundTag> OrAP,
         datalog::AndAnnotationPolicyConcept<GroundTag> AndAP,
         datalog::TerminationPolicyConcept<GroundTag> TP,
         datalog::RuleCostPolicyConcept<GroundTag> CP>
datalog::Cost
RPGBase<GroundTag, Derived, OrAP, AndAP, TP, CP>::get_atom_cost(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> atom) const noexcept
{
    const auto* annotation = m_workspace.and_annot.find(atom);
    return annotation ? datalog::get_cost(*annotation) : datalog::Cost(0);
}

template<typename Derived,
         datalog::OrAnnotationPolicyConcept<GroundTag> OrAP,
         datalog::AndAnnotationPolicyConcept<GroundTag> AndAP,
         datalog::TerminationPolicyConcept<GroundTag> TP,
         datalog::RuleCostPolicyConcept<GroundTag> CP>
datalog::Cost RPGBase<GroundTag, Derived, OrAP, AndAP, TP, CP>::get_goal_cost() const noexcept
{
    return m_workspace.tp.get_total_cost(m_workspace.facts, m_workspace.and_annot);
}

}

#endif
