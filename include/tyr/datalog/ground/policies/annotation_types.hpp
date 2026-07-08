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

#ifndef TYR_DATALOG_GROUND_POLICIES_ANNOTATION_TYPES_HPP_
#define TYR_DATALOG_GROUND_POLICIES_ANNOTATION_TYPES_HPP_

#include "tyr/datalog/policies/annotation_types.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>

namespace tyr::datalog
{

template<>
struct NumericSupport<GroundTag>
{
    ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag> term;
    ygg::ClosedInterval<ygg::float_t> interval;
    Cost cost;

    auto get_term() const noexcept { return term; }
    auto get_interval() const noexcept { return interval; }
    auto get_cost() const noexcept { return cost; }

    auto identifying_members() const noexcept { return std::make_tuple(term, lower(interval), upper(interval), cost); }
};

template<>
struct WitnessAnnotation<GroundTag>
{
    using Metric = ygg::ClosedInterval<ygg::float_t>;
    using NumericSupports = std::vector<NumericSupport<GroundTag>>;

    WitnessAnnotation(::tyr::formalism::datalog::GroundRuleView rule_, Cost cost_) : rule(rule_), metric(), cost(cost_) {}

    WitnessAnnotation(::tyr::formalism::datalog::GroundRuleView rule_, Metric metric_, Cost cost_) : rule(rule_), metric(metric_), cost(cost_) {}

    WitnessAnnotation(::tyr::formalism::datalog::GroundRuleView rule_, Metric metric_, Cost cost_, NumericSupports numeric_supports_) :
        rule(rule_),
        metric(metric_),
        cost(cost_),
        numeric_supports(std::move(numeric_supports_))
    {
        std::sort(numeric_supports.begin(), numeric_supports.end(), ygg::Less<NumericSupport<GroundTag>> {});
    }

    auto get_rule() const noexcept { return rule; }
    auto get_metric() const noexcept { return metric; }
    auto get_cost() const noexcept { return cost; }
    const auto& get_numeric_supports() const noexcept { return numeric_supports; }

    auto identifying_members() const noexcept { return std::tie(rule, metric, cost, numeric_supports); }

private:
    ::tyr::formalism::datalog::GroundRuleView rule;
    Metric metric;
    Cost cost;
    NumericSupports numeric_supports;
};

template<>
class PredicateAnnotationMap<GroundTag>
{
public:
    using Atom = ::tyr::formalism::datalog::GroundAtomView<::tyr::formalism::FluentTag>;

    void clear() noexcept { annotations.clear(); }

    void insert_or_assign(Atom atom, Annotation<GroundTag> annotation) { annotations.insert_or_assign(atom, std::move(annotation)); }

    const Annotation<GroundTag>* find(Atom atom) const noexcept
    {
        const auto it = annotations.find(atom);
        return it == annotations.end() ? nullptr : &it->second;
    }

    Annotation<GroundTag>* find(Atom atom) noexcept
    {
        const auto it = annotations.find(atom);
        return it == annotations.end() ? nullptr : &it->second;
    }

private:
    ygg::UnorderedMap<Atom, Annotation<GroundTag>> annotations;
};

template<>
class NumericIntervalAnnotations<GroundTag>
{
public:
    using Binding = ::tyr::formalism::datalog::GroundFunctionTermView<::tyr::formalism::FluentTag>;
    using Relation = ::tyr::formalism::datalog::FunctionView<::tyr::formalism::FluentTag>;
    using Term = ygg::Index<::tyr::formalism::datalog::GroundFunctionTerm<::tyr::formalism::FluentTag>>;
    using Entry = NumericIntervalAnnotation<GroundTag>;
    using Entries = std::vector<Entry>;
    using TermPartitions = ygg::UnorderedMap<Term, Entries>;
    using Partitions = ygg::UnorderedMap<Relation, TermPartitions>;

    void clear() noexcept
    {
        m_size = 0;
        for (auto& [_, term_partitions] : m_partitions)
            for (auto& [_, entries] : term_partitions)
                entries.clear();
    }

    size_t size() const noexcept { return m_size; }

    const Partitions& partitions() const noexcept { return m_partitions; }

    const Annotation<GroundTag>* find(Binding binding) const noexcept
    {
        const auto* entries = find_entries(binding);
        return (!entries || entries->empty()) ? nullptr : &entries->back().annotation;
    }

    Annotation<GroundTag>* find(Binding binding) noexcept
    {
        auto* entries = find_entries(binding);
        return (!entries || entries->empty()) ? nullptr : &entries->back().annotation;
    }

    const Annotation<GroundTag>* find(Binding binding, ygg::ClosedInterval<ygg::float_t> interval) const noexcept
    {
        const auto* entries = find_entries(binding);
        if (!entries)
            return nullptr;

        for (const auto& entry : *entries)
            if (entry.interval == interval)
                return &entry.annotation;

        return nullptr;
    }

    void insert(Binding binding, ygg::ClosedInterval<ygg::float_t> interval, Annotation<GroundTag> annotation)
    {
        if (empty(interval))
            return;

        auto entry = Entry { interval, std::move(annotation) };
        auto& entries = m_partitions[binding.get_function()][binding.get_index()];
        entries.insert(std::upper_bound(entries.begin(), entries.end(), entry, ygg::Less<Entry> {}), std::move(entry));
        ++m_size;
    }

private:
    const Entries* find_entries(Binding binding) const noexcept
    {
        const auto relation_it = m_partitions.find(binding.get_function());
        if (relation_it == m_partitions.end())
            return nullptr;

        const auto term_it = relation_it->second.find(binding.get_index());
        return term_it == relation_it->second.end() ? nullptr : &term_it->second;
    }

    Entries* find_entries(Binding binding) noexcept
    {
        const auto relation_it = m_partitions.find(binding.get_function());
        if (relation_it == m_partitions.end())
            return nullptr;

        const auto term_it = relation_it->second.find(binding.get_index());
        return term_it == relation_it->second.end() ? nullptr : &term_it->second;
    }

    Partitions m_partitions;
    size_t m_size = 0;
};

template<>
struct AndAnnotationContext<GroundTag>
{
    ygg::ClosedInterval<ygg::float_t> metric;
    Cost current_cost;
    std::vector<NumericSupport<GroundTag>> numeric_supports;
    ::tyr::formalism::datalog::GroundRuleView rule;
    const SelectedPredicateAnnotations<GroundTag>& program_and_annot;
};

using GroundWitnessAnnotation = WitnessAnnotation<GroundTag>;
using GroundBaseAnnotation = BaseAnnotation<GroundTag>;
using GroundAnnotation = Annotation<GroundTag>;
using GroundSelectedPredicateAnnotations = SelectedPredicateAnnotations<GroundTag>;
using GroundSelectedFunctionAnnotations = SelectedFunctionAnnotations<GroundTag>;
using GroundAndAnnotationContext = AndAnnotationContext<GroundTag>;
using GroundCostUpdate = CostUpdate<GroundTag>;

}

#endif
