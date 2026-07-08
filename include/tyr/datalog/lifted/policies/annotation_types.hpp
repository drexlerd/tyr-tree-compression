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

#ifndef TYR_DATALOG_LIFTED_POLICIES_ANNOTATION_TYPES_HPP_
#define TYR_DATALOG_LIFTED_POLICIES_ANNOTATION_TYPES_HPP_

#include "tyr/datalog/policies/annotation_types.hpp"
#include "tyr/formalism/datalog/repository.hpp"

#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/containers/block_array_ordering.hpp>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/semantics/comparators.hpp>

namespace tyr::datalog
{

template<>
struct NumericSupport<LiftedTag>
{
    ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag> binding;
    ygg::ClosedInterval<ygg::float_t> interval;
    Cost cost;

    auto get_binding() const noexcept { return binding; }
    auto get_interval() const noexcept { return interval; }
    auto get_cost() const noexcept { return cost; }

    auto identifying_members() const noexcept { return std::make_tuple(binding, lower(interval), upper(interval)); }
};

template<>
struct WitnessAnnotation<LiftedTag>
{
public:
    using Metric = ygg::ClosedInterval<ygg::float_t>;
    using NumericSupports = std::vector<NumericSupport<LiftedTag>>;

    WitnessAnnotation(::tyr::formalism::datalog::RuleBindingView rule_row, Cost cost) : m_rule_row(rule_row), m_metric(), m_cost(cost) {}

    WitnessAnnotation(::tyr::formalism::datalog::RuleBindingView rule_row, Metric metric, Cost cost) : m_rule_row(rule_row), m_metric(metric), m_cost(cost) {}

    WitnessAnnotation(::tyr::formalism::datalog::RuleBindingView rule_row, Metric metric, Cost cost, NumericSupports numeric_supports) :
        m_rule_row(rule_row),
        m_metric(metric),
        m_cost(cost),
        m_numeric_supports(std::move(numeric_supports))
    {
    }

    auto get_rule_row() const noexcept { return m_rule_row; }
    auto get_metric() const noexcept { return m_metric; }
    auto get_cost() const noexcept { return m_cost; }
    const auto& get_numeric_supports() const noexcept { return m_numeric_supports; }

    auto identifying_members() const noexcept { return std::tie(m_rule_row); }

private:
    ::tyr::formalism::datalog::RuleBindingView m_rule_row;
    Metric m_metric;
    Cost m_cost;
    NumericSupports m_numeric_supports;
};

template<>
class PredicateAnnotationMap<LiftedTag>
{
public:
    using Binding = ::tyr::formalism::datalog::PredicateBindingView<::tyr::formalism::FluentTag>;
    using Relation = ::tyr::formalism::datalog::PredicateView<::tyr::formalism::FluentTag>;
    using Row = ygg::Index<::tyr::formalism::Row>;
    using Inner = ygg::UnorderedMap<Row, Annotation<LiftedTag>>;
    using Outer = ygg::UnorderedMap<Relation, Inner>;

    void clear() noexcept
    {
        for (auto& [_, annotations] : m_annotations)
            annotations.clear();
    }

    void insert_or_assign(Binding binding, Annotation<LiftedTag> annotation)
    {
        m_annotations[binding.get_relation()].insert_or_assign(binding.get_index().row, std::move(annotation));
    }

    const Annotation<LiftedTag>* find(Binding binding) const noexcept
    {
        const auto relation_it = m_annotations.find(binding.get_relation());
        if (relation_it == m_annotations.end())
            return nullptr;

        const auto annotation_it = relation_it->second.find(binding.get_index().row);
        return annotation_it == relation_it->second.end() ? nullptr : &annotation_it->second;
    }

    Annotation<LiftedTag>* find(Binding binding) noexcept
    {
        const auto relation_it = m_annotations.find(binding.get_relation());
        if (relation_it == m_annotations.end())
            return nullptr;

        const auto annotation_it = relation_it->second.find(binding.get_index().row);
        return annotation_it == relation_it->second.end() ? nullptr : &annotation_it->second;
    }

private:
    Outer m_annotations;
};

template<>
class NumericIntervalAnnotations<LiftedTag>
{
public:
    using Binding = ::tyr::formalism::datalog::FunctionBindingView<::tyr::formalism::FluentTag>;
    using Relation = ::tyr::formalism::datalog::FunctionView<::tyr::formalism::FluentTag>;
    using Row = ygg::Index<::tyr::formalism::Row>;
    using Entry = NumericIntervalAnnotation<LiftedTag>;
    using Entries = std::vector<Entry>;
    using RowPartitions = ygg::UnorderedMap<Row, Entries>;
    using Partitions = ygg::UnorderedMap<Relation, RowPartitions>;

    void clear() noexcept
    {
        m_size = 0;
        for (auto& [_, row_partitions] : m_partitions)
            for (auto& [_, entries] : row_partitions)
                entries.clear();
    }

    size_t size() const noexcept { return m_size; }

    const Partitions& partitions() const noexcept { return m_partitions; }

    const Annotation<LiftedTag>* find(Binding binding) const noexcept
    {
        const auto* entries = find_entries(binding);
        return (!entries || entries->empty()) ? nullptr : &entries->back().annotation;
    }

    Annotation<LiftedTag>* find(Binding binding) noexcept
    {
        auto* entries = find_entries(binding);
        return (!entries || entries->empty()) ? nullptr : &entries->back().annotation;
    }

    const Annotation<LiftedTag>* find(Binding binding, ygg::ClosedInterval<ygg::float_t> interval) const noexcept
    {
        const auto* entries = find_entries(binding);
        if (!entries)
            return nullptr;

        for (const auto& entry : *entries)
            if (entry.interval == interval)
                return &entry.annotation;

        return nullptr;
    }

    void insert(Binding binding, ygg::ClosedInterval<ygg::float_t> interval, Annotation<LiftedTag> annotation)
    {
        if (empty(interval))
            return;

        // Entries are cost-sorted; equal-cost entries are further ordered canonically (interval bounds,
        // then witness rule binding) instead of insertion order, so the candidate order downstream
        // consumers see is independent of the platform-unspecified processing order.
        const auto less = [](const Entry& lhs, const Entry& rhs)
        {
            const auto lhs_cost = get_cost(lhs.annotation);
            const auto rhs_cost = get_cost(rhs.annotation);
            if (lhs_cost != rhs_cost)
                return lhs_cost < rhs_cost;
            if (lower(lhs.interval) != lower(rhs.interval))
                return lower(lhs.interval) < lower(rhs.interval);
            if (upper(lhs.interval) != upper(rhs.interval))
                return upper(lhs.interval) < upper(rhs.interval);
            const auto* lhs_witness = std::get_if<WitnessAnnotation<LiftedTag>>(&lhs.annotation);
            const auto* rhs_witness = std::get_if<WitnessAnnotation<LiftedTag>>(&rhs.annotation);
            if (!lhs_witness || !rhs_witness)
                return !lhs_witness && rhs_witness;  ///< BaseAnnotation (initial fact) sorts first
            return ygg::Less<::tyr::formalism::datalog::RuleBindingView> {}(lhs_witness->get_rule_row(), rhs_witness->get_rule_row());
        };

        auto entry = Entry { interval, std::move(annotation) };
        auto& entries = m_partitions[binding.get_relation()][binding.get_index().row];
        entries.insert(std::upper_bound(entries.begin(), entries.end(), entry, less), std::move(entry));
        ++m_size;
    }

private:
    const Entries* find_entries(Binding binding) const noexcept
    {
        const auto relation_it = m_partitions.find(binding.get_relation());
        if (relation_it == m_partitions.end())
            return nullptr;

        const auto row_it = relation_it->second.find(binding.get_index().row);
        return row_it == relation_it->second.end() ? nullptr : &row_it->second;
    }

    Entries* find_entries(Binding binding) noexcept
    {
        const auto relation_it = m_partitions.find(binding.get_relation());
        if (relation_it == m_partitions.end())
            return nullptr;

        const auto row_it = relation_it->second.find(binding.get_index().row);
        return row_it == relation_it->second.end() ? nullptr : &row_it->second;
    }

    Partitions m_partitions;
    size_t m_size = 0;
};

template<>
struct AndAnnotationContext<LiftedTag>
{
    Cost current_cost;
    std::vector<NumericSupport<LiftedTag>> numeric_supports;
    ::tyr::formalism::datalog::RuleView rule;
    ::tyr::formalism::datalog::RuleBindingView rule_binding;
    Cost metric_effect_cost;
    ::tyr::formalism::datalog::ConjunctiveConditionView witness_condition;
    const NumericSupportSelector& numeric_support_selector;
    NumericSupportSelectorWorkspace& numeric_support_selector_workspace;
    const SelectedPredicateAnnotations<LiftedTag>& program_and_annot;
    const SelectedFunctionAnnotations<LiftedTag>& program_numeric_and_annot;
    ::tyr::formalism::datalog::GrounderContext& delta_context;
    ::tyr::formalism::datalog::GrounderContext& iteration_context;
};

using LiftedWitnessAnnotation = WitnessAnnotation<LiftedTag>;
using LiftedBaseAnnotation = BaseAnnotation<LiftedTag>;
using LiftedAnnotation = Annotation<LiftedTag>;
using LiftedSelectedPredicateAnnotations = SelectedPredicateAnnotations<LiftedTag>;
using LiftedSelectedFunctionAnnotations = SelectedFunctionAnnotations<LiftedTag>;
using LiftedAndAnnotationContext = AndAnnotationContext<LiftedTag>;
using LiftedCostUpdate = CostUpdate<LiftedTag>;

}

#endif
