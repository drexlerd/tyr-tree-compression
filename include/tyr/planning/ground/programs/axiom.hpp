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

#ifndef TYR_PLANNING_GROUND_PROGRAMS_AXIOM_HPP_
#define TYR_PLANNING_GROUND_PROGRAMS_AXIOM_HPP_

#include "tyr/formalism/datalog/repository.hpp"
#include "tyr/formalism/datalog/views.hpp"
#include "tyr/formalism/planning/views.hpp"
#include "tyr/planning/programs/axiom.hpp"
#include "tyr/planning/programs/translation_context.hpp"

namespace tyr::planning
{

template<>
class AxiomEvaluatorProgram<GroundTag>
{
public:
    explicit AxiomEvaluatorProgram(::tyr::formalism::planning::FDRTaskView task);

    const TranslationContext& get_translation_context() const noexcept;
    ::tyr::formalism::datalog::GroundProgramView get_ground_program() const noexcept;
    const ::tyr::formalism::datalog::Repository& get_program_repository() const noexcept;

private:
    TranslationContext m_translation_context;
    ::tyr::formalism::datalog::RepositoryFactoryPtr m_repository_factory;
    ::tyr::formalism::datalog::RepositoryPtr m_repository;
    ::tyr::formalism::datalog::GroundProgramView m_ground_program;
};

}

#endif
