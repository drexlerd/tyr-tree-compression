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

#ifndef TYR_PLANNING_LIFTED_PROGRAMS_AXIOM_HPP_
#define TYR_PLANNING_LIFTED_PROGRAMS_AXIOM_HPP_

#include "tyr/datalog/program_context.hpp"
#include "tyr/datalog/lifted/workspaces/program.hpp"
#include "tyr/formalism/planning/views.hpp"
#include "tyr/planning/programs/axiom.hpp"
#include "tyr/planning/programs/translation_context.hpp"

namespace tyr::planning
{

template<>
class AxiomEvaluatorProgram<LiftedTag>
{
public:
    explicit AxiomEvaluatorProgram(::tyr::formalism::planning::TaskView task);

    const TranslationContext& get_translation_context() const noexcept;
    datalog::ProgramContext& get_program_context() noexcept;
    const datalog::ProgramContext& get_program_context() const noexcept;
    const datalog::ConstProgramWorkspace<LiftedTag>& get_const_program_workspace() const noexcept;

private:
    TranslationContext m_translation_context;
    datalog::ProgramContext m_program_context;
    datalog::ConstProgramWorkspace<LiftedTag> m_program_workspace;
};

}

#endif
