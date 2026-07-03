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

#ifndef TYR_PLANNING_GROUND_HPP_
#define TYR_PLANNING_GROUND_HPP_

#include "tyr/formalism/planning/declarations.hpp"  // for OverlayRepos...
#include "tyr/formalism/planning/fdr_context.hpp"
#include "tyr/formalism/planning/planning_fdr_task.hpp"
#include "tyr/formalism/planning/views.hpp"  // for ygg::View
#include "tyr/planning/declarations.hpp"
#include "tyr/planning/ground/match_tree/match_tree.hpp"  // for Matc...
#include "tyr/planning/ground/programs/action.hpp"
#include "tyr/planning/ground/programs/axiom.hpp"
#include "tyr/planning/ground/programs/rpg.hpp"
#include "tyr/planning/task.hpp"

#include <boost/dynamic_bitset.hpp>                 // for dynamic_bitset
#include <limits>                                   // for numeric_limits
#include <optional>                                 // for optional
#include <stddef.h>                                 // for size_t
#include <vector>                                   // for vector
#include <yggdrasil/containers/dynamic_bitset.hpp>  // for test
#include <yggdrasil/containers/vector.hpp>          // for get
#include <yggdrasil/core/config.hpp>                // for ygg::float_t, ygg::uint_t

namespace tyr::planning
{

template<>
class Task<GroundTag>
{
public:
    explicit Task(::tyr::formalism::planning::PlanningFDRTask task);

    template<::tyr::formalism::FactKind T>
    size_t get_num_atoms() const noexcept;
    size_t get_num_actions() const noexcept;
    size_t get_num_axioms() const noexcept;

    const auto& get_static_atoms_bitset() const noexcept { return m_static_atoms_bitset; }
    const auto& get_static_numeric_variables() const noexcept { return m_static_numeric_variables; }
    bool test(ygg::Index<::tyr::formalism::planning::GroundAtom<::tyr::formalism::StaticTag>> index) const
    {
        return ygg::test(ygg::uint_t(index), m_static_atoms_bitset);
    }
    ygg::float_t get(ygg::Index<::tyr::formalism::planning::GroundFunctionTerm<::tyr::formalism::StaticTag>> index) const noexcept
    {
        return ygg::get(ygg::uint_t(index), m_static_numeric_variables, std::numeric_limits<ygg::float_t>::quiet_NaN());
    }

    const auto& get_formalism_task() const noexcept { return m_task; }
    const auto& get_domain() const noexcept { return m_task.get_domain(); }
    auto get_task() const noexcept { return m_task.get_task(); }
    const auto& get_fdr_context() const noexcept { return m_task.get_fdr_context(); }
    const auto& get_repository() const noexcept { return m_task.get_repository(); }
    bool has_axioms() const noexcept { return !get_task().get_ground_axioms().empty(); }

    const auto& get_action_match_tree() const noexcept { return m_action_match_tree; }
    std::optional<::tyr::formalism::planning::GroundActionView> find_ground_action(::tyr::formalism::planning::ActionBindingView binding) const;
    ::tyr::formalism::planning::GroundActionView get_ground_action(::tyr::formalism::planning::ActionBindingView binding) const;
    const auto& get_axiom_match_tree_strata() const noexcept { return m_axiom_match_tree_strata; }

    auto& get_axiom_program() noexcept { return m_axiom_program; }
    const auto& get_axiom_program() const noexcept { return m_axiom_program; }
    auto& get_action_program() noexcept { return m_action_program; }
    const auto& get_action_program() const noexcept { return m_action_program; }
    auto& get_rpg_program() noexcept { return m_rpg_program; }
    const auto& get_rpg_program() const noexcept { return m_rpg_program; }

private:
    ::tyr::formalism::planning::PlanningFDRTask m_task;

    boost::dynamic_bitset<> m_static_atoms_bitset;
    std::vector<ygg::float_t> m_static_numeric_variables;

    match_tree::MatchTreePtr<::tyr::formalism::planning::GroundAction> m_action_match_tree;
    ygg::UnorderedMap<::tyr::formalism::planning::ActionBindingView, ::tyr::formalism::planning::GroundActionView> m_action_binding_to_ground_action;

    std::vector<match_tree::MatchTreePtr<::tyr::formalism::planning::GroundAxiom>> m_axiom_match_tree_strata;

    AxiomEvaluatorProgram<GroundTag> m_axiom_program;

    ApplicableActionProgram<GroundTag> m_action_program;

    RPGProgram<GroundTag> m_rpg_program;
};

}

#endif
