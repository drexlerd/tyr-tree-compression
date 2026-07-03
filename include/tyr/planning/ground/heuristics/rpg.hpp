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
#include "tyr/planning/declarations.hpp"
#include "tyr/planning/ground/state_view.hpp"
#include "tyr/planning/ground_task.hpp"
#include "tyr/planning/heuristic.hpp"
#include "tyr/planning/heuristics/rpg.hpp"

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

private:
    TaskPtr<GroundTag> m_task;
    ygg::ExecutionContextPtr m_execution_context;
    datalog::ProgramWorkspace<GroundTag, OrAP, AndAP, TP, CP> m_workspace;
};

}

#endif
