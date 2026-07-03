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

#ifndef TYR_DATALOG_GROUND_CONTEXTS_PROGRAM_HPP_
#define TYR_DATALOG_GROUND_CONTEXTS_PROGRAM_HPP_

#include "tyr/datalog/contexts/program.hpp"
#include "tyr/datalog/ground/policies/annotation.hpp"
#include "tyr/datalog/ground/policies/cost.hpp"
#include "tyr/datalog/ground/policies/termination.hpp"
#include "tyr/datalog/ground/workspaces/program.hpp"
#include "tyr/datalog/policies/annotation_concept.hpp"
#include "tyr/datalog/policies/cost_concept.hpp"
#include "tyr/datalog/policies/termination_concept.hpp"

#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/containers/variant.hpp>

namespace tyr::datalog
{

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
struct ProgramExecutionContext<GroundTag, OrAP, AndAP, TP, CP>
{
    class In
    {
    public:
        explicit In(const ConstProgramWorkspace<GroundTag>& cws) : m_cws(cws) {}

        auto program() const noexcept { return m_cws.program; }
        const auto& fluent_precondition_to_rules() const noexcept { return m_cws.fluent_precondition_to_rules; }

    private:
        const ConstProgramWorkspace<GroundTag>& m_cws;
    };

    class Out
    {
    public:
        explicit Out(ProgramWorkspace<GroundTag, OrAP, AndAP, TP, CP>& ws) : m_ws(ws) {}

        auto& facts() noexcept { return m_ws.facts; }
        const auto& facts() const noexcept { return m_ws.facts; }
        auto& or_ap() noexcept { return m_ws.or_ap; }
        const auto& or_ap() const noexcept { return m_ws.or_ap; }
        auto& and_ap() noexcept { return m_ws.and_ap; }
        const auto& and_ap() const noexcept { return m_ws.and_ap; }
        auto& and_annot() noexcept { return m_ws.and_annot; }
        const auto& and_annot() const noexcept { return m_ws.and_annot; }
        auto& tp() noexcept { return m_ws.tp; }
        const auto& tp() const noexcept { return m_ws.tp; }
        auto& cost_policy() noexcept { return m_ws.cost_policy; }
        const auto& cost_policy() const noexcept { return m_ws.cost_policy; }
        auto& rules() noexcept { return m_ws.rules; }
        const auto& rules() const noexcept { return m_ws.rules; }
        auto& unsatisfied_counts() noexcept { return m_ws.rules.unsatisfied_counts; }
        const auto& unsatisfied_counts() const noexcept { return m_ws.rules.unsatisfied_counts; }
        auto& fired_rules() noexcept { return m_ws.rules.fired_rules; }
        const auto& fired_rules() const noexcept { return m_ws.rules.fired_rules; }
        auto& queue_storage() noexcept { return m_ws.rules.queue_storage; }
        const auto& queue_storage() const noexcept { return m_ws.rules.queue_storage; }
        auto& fluent_atoms() noexcept { return m_ws.facts.fluent_atoms; }
        const auto& fluent_atoms() const noexcept { return m_ws.facts.fluent_atoms; }
        auto& statistics() noexcept { return m_ws.statistics; }
        const auto& statistics() const noexcept { return m_ws.statistics; }

    private:
        ProgramWorkspace<GroundTag, OrAP, AndAP, TP, CP>& m_ws;
    };

    ProgramExecutionContext(ProgramWorkspace<GroundTag, OrAP, AndAP, TP, CP>& ws, const ConstProgramWorkspace<GroundTag>& cws) : m_in(cws), m_out(ws)
    {
        clear();
    }

    void clear()
    {
        m_out.fluent_atoms().clear();
        for (const auto fact : m_in.program().template get_atoms<::tyr::formalism::FluentTag>())
            m_out.fluent_atoms().insert(fact);
        initialize_from_current_fluent_atoms();
    }

    void initialize(ygg::UnorderedSet<::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>> fluent_atoms)
    {
        m_out.fluent_atoms() = std::move(fluent_atoms);
        initialize_from_current_fluent_atoms();
    }

    const auto& in() const noexcept { return m_in; }
    auto& out() noexcept { return m_out; }
    const auto& out() const noexcept { return m_out; }

private:
    template<typename Index>
    static size_t position(Index index) noexcept
    {
        return static_cast<size_t>(index.get_value());
    }

    template<typename T, typename Index>
    static decltype(auto) at(std::vector<T>& vector, Index index) noexcept
    {
        return vector[position(index)];
    }

    template<typename T, typename Index>
    static decltype(auto) at(const std::vector<T>& vector, Index index) noexcept
    {
        return vector[position(index)];
    }

    bool is_static_fact_true(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::StaticTag> fact) const noexcept
    {
        for (const auto atom : m_in.program().template get_atoms<::tyr::formalism::StaticTag>())
            if (atom.get_index() == fact.get_index())
                return true;
        return false;
    }

    bool is_fluent_fact_true(::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag> fact) const noexcept
    {
        return m_out.fluent_atoms().contains(fact);
    }

    void initialize_annotations()
    {
        m_out.and_annot().clear();
        m_out.and_ap().clear_achievers();
        m_out.tp().reset();
        for (const auto fact : m_out.fluent_atoms())
            m_out.or_ap().initialize_annotation(fact, m_out.and_annot());
    }

    void initialize_from_current_fluent_atoms()
    {
        initialize_annotations();
        m_out.unsatisfied_counts().clear();
        m_out.fired_rules().clear();
        for (const auto rule : m_in.program().get_ground_rules())
        {
            const auto rule_index = rule.get_index();
            if (position(rule_index) >= m_out.unsatisfied_counts().size())
            {
                m_out.unsatisfied_counts().resize(position(rule_index) + 1, 0);
                m_out.fired_rules().resize(position(rule_index) + 1, false);
            }

            auto unsatisfied_count = ygg::uint_t(0);
            for (const auto literal : rule.get_body().template get_literals<::tyr::formalism::StaticTag>())
                if (literal.get_polarity() && !is_static_fact_true(literal.get_atom()))
                    ++unsatisfied_count;
            for (const auto literal : rule.get_body().template get_literals<::tyr::formalism::FluentTag>())
                if (!is_fluent_fact_true(literal.get_atom()))
                    ++unsatisfied_count;
            at(m_out.unsatisfied_counts(), rule_index) = unsatisfied_count;
            at(m_out.fired_rules(), rule_index) = false;
        }
        m_out.queue_storage().clear();
        m_out.statistics() = GroundQueueStatistics {};
    }

    In m_in;
    Out m_out;
};

}

#endif
