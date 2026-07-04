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

#include "tyr/datalog/ground/queue.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <map>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include <yggdrasil/containers/associative_containers.hpp>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/config.hpp>
#include <yggdrasil/semantics/comparators.hpp>

namespace tyr::datalog
{
namespace
{
namespace f = ::tyr::formalism;
namespace fd = ::tyr::formalism::datalog;

template<typename Index>
size_t position(Index index) noexcept
{
    return static_cast<size_t>(index.get_value());
}

template<typename T, typename Index>
decltype(auto) at(std::vector<T>& vector, Index index) noexcept
{
    return vector[position(index)];
}

template<typename T, typename Index>
decltype(auto) at(const std::vector<T>& vector, Index index) noexcept
{
    return vector[position(index)];
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
using GroundCtx = ProgramExecutionContext<GroundTag, OrAP, AndAP, TP, CP>;

bool has_atom_head(fd::GroundRuleView rule) noexcept;

struct PendingNumericBuckets
{
    using Term = fd::GroundFunctionTermView<f::FluentTag>;
    using Interval = ygg::ClosedInterval<ygg::float_t>;
    using Bucket = ygg::UnorderedMap<Term, Interval>;

    bool is_empty() const noexcept { return buckets.empty(); }

    Cost min_cost() const noexcept { return buckets.empty() ? std::numeric_limits<Cost>::max() : buckets.begin()->first; }

    bool insert(Cost cost, Term term, Interval interval)
    {
        if (empty(interval))
            return false;

        auto& bucket = buckets[cost];
        const auto it = bucket.find(term);
        if (it == bucket.end())
        {
            bucket.emplace(term, interval);
            return true;
        }

        if (subset(interval, it->second))
            return false;

        it->second = hull(it->second, interval);
        return true;
    }

    Bucket take(Cost cost)
    {
        auto it = buckets.find(cost);
        if (it == buckets.end())
            return Bucket {};

        auto bucket = std::move(it->second);
        buckets.erase(it);
        return bucket;
    }

    std::map<Cost, Bucket> buckets;
};

template<typename Facts>
ygg::ClosedInterval<ygg::float_t> find_interval(const Facts& facts, fd::GroundFunctionTermView<f::FluentTag> term) noexcept
{
    const auto it = facts.fluent_fterm_intervals.find(term);
    return it == facts.fluent_fterm_intervals.end() ? ygg::ClosedInterval<ygg::float_t>() : it->second;
}

using Metric = ygg::ClosedInterval<ygg::float_t>;
using NumericSelectionEntry = GroundNumericSupportSelectorWorkspace::SelectionEntry;

void append_numeric_supports(std::vector<NumericSupport<GroundTag>>& supports, const std::vector<NumericSelectionEntry>& selection)
{
    for (const auto& entry : selection)
        supports.push_back(NumericSupport<GroundTag> { entry.term, entry.interval, entry.cost });
}

template<typename Op>
ygg::ClosedInterval<ygg::float_t> apply_numeric_effect(Op, ygg::ClosedInterval<ygg::float_t> lhs, ygg::ClosedInterval<ygg::float_t> rhs);

template<AndAnnotationPolicyConcept<GroundTag> AndAP,
         OrAnnotationPolicyConcept<GroundTag> OrAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
GroundNumericSupportSelector make_numeric_support_selector(const GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    return GroundNumericSupportSelector(ctx.out().facts(), ctx.out().numeric_and_annot(), !requires { AndAP::agg; });
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
Metric aggregate_numeric_selection_metric(Metric metric, const std::vector<NumericSelectionEntry>& selection, const GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    for (const auto& entry : selection)
    {
        if (entry.annotation)
        {
            metric = aggregate_metric_support(metric, get_metric(entry.annotation->annotation));
            continue;
        }

        const auto relation_it = ctx.out().numeric_and_annot().partitions().find(entry.term.get_function());
        if (relation_it == ctx.out().numeric_and_annot().partitions().end())
            continue;

        const auto term_it = relation_it->second.find(entry.term.get_index());
        if (term_it == relation_it->second.end())
            continue;

        const auto current = find_interval(ctx.out().facts(), entry.term);
        if (empty(current))
            continue;

        for (const auto& candidate : term_it->second)
        {
            if (get_cost(candidate.annotation) <= entry.cost && subset(candidate.interval, entry.interval) && subset(candidate.interval, current))
                metric = aggregate_metric_support(metric, get_metric(candidate.annotation));
        }
    }
    return metric;
}

template<AndAnnotationPolicyConcept<GroundTag> AndAP,
         OrAnnotationPolicyConcept<GroundTag> OrAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
Metric aggregate_body_metric(fd::GroundRuleView rule, const GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    auto metric = Metric {};
    if constexpr (requires { AndAP::agg; })
    {
        for (const auto literal : rule.get_body().template get_literals<f::FluentTag>())
        {
            if (!literal.get_polarity())
                continue;
            const auto* annotation = ctx.out().and_annot().find(literal.get_atom());
            assert(annotation && "enabled ground rule has a positive fluent body atom without a cost annotation");
            metric = aggregate_metric_support(metric, get_metric(*annotation));
        }

        auto selector = make_numeric_support_selector<AndAP>(ctx);
        auto selection = std::vector<NumericSelectionEntry> {};
        for (const auto numeric_constraint : rule.get_body().get_numeric_constraints())
        {
            if (selector.get_constraint_cost(numeric_constraint, selection, typename std::decay_t<decltype(AndAP::agg)> {}) != std::numeric_limits<Cost>::max())
                metric = aggregate_numeric_selection_metric(metric, selection, ctx);
        }
    }
    return metric;
}

template<typename AggregationFunction>
Cost aggregate_selection_cost(Cost cost, const std::vector<NumericSelectionEntry>& selection, AggregationFunction agg)
{
    for (const auto& entry : selection)
        cost = agg(cost, entry.cost);
    return cost;
}

inline Cost clamp_metric_delta(ygg::float_t value) noexcept
{
    if (!std::isfinite(value))
        return Cost(0);
    return Cost(std::max(value, ygg::float_t(0)));
}

template<f::NumericEffectOpKind Op>
Cost metric_effect_delta(fd::GroundNumericEffectView<Op, f::FluentTag> effect,
                         const GroundNumericSupportSelector& selector,
                         std::vector<NumericSelectionEntry>& selection)
{
    const auto rhs = selector.evaluate_effect_expression(effect.get_fexpr(), selection);
    if (empty(rhs))
        return std::numeric_limits<Cost>::max();

    if constexpr (std::is_same_v<Op, f::Increase>)
    {
        return clamp_metric_delta(lower(rhs));
    }
    else if constexpr (std::is_same_v<Op, f::Decrease>)
    {
        return Cost(0);
    }
    else
    {
        const auto lhs = selector.select_fluent_interval(effect.get_fterm(), selection);
        if (empty(lhs))
            return std::numeric_limits<Cost>::max();
        const auto next = apply_numeric_effect(Op {}, lhs, rhs);
        if (empty(next))
            return std::numeric_limits<Cost>::max();
        return clamp_metric_delta(lower(next) - upper(lhs));
    }
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
Cost aggregate_metric_effect_cost(fd::GroundRuleView rule, const GroundCtx<OrAP, AndAP, TP, CP>& ctx, std::vector<NumericSelectionEntry>& selection)
{
    auto selector = make_numeric_support_selector<AndAP>(ctx);
    selection.clear();

    auto delta = Cost(0);
    for (const auto metric_effect : rule.get_metric_effects())
    {
        const auto effect_delta = ygg::visit([&](auto&& effect) { return metric_effect_delta(effect, selector, selection); }, metric_effect.get_variant());
        if (effect_delta == std::numeric_limits<Cost>::max())
            return effect_delta;
        delta += effect_delta;
    }

    const auto used_cost = ctx.out().cost_policy().get_cost(rule);
    return delta <= used_cost ? Cost(0) : delta - used_cost;
}

template<AndAnnotationPolicyConcept<GroundTag> AndAP>
Metric add_metric_delta(Metric metric, Cost delta)
{
    if constexpr (requires { AndAP::agg; })
    {
        if (delta == Cost(0))
            return metric;
        if (empty(metric))
            return Metric(delta, delta);
        return Metric(lower(metric) + delta, upper(metric) + delta);
    }
    return metric;
}

template<AndAnnotationPolicyConcept<GroundTag> AndAP,
         OrAnnotationPolicyConcept<GroundTag> OrAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
Cost aggregate_body_cost(fd::GroundRuleView rule, const GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    if constexpr (requires { AndAP::agg; })
    {
        auto cost = std::decay_t<decltype(AndAP::agg)>::identity();
        for (const auto literal : rule.get_body().template get_literals<f::FluentTag>())
        {
            if (!literal.get_polarity())
                continue;
            const auto* annotation = ctx.out().and_annot().find(literal.get_atom());
            assert(annotation && "enabled ground rule has a positive fluent body atom without a cost annotation");
            cost = AndAP::agg(cost, get_cost(*annotation));
        }

        auto selector = make_numeric_support_selector<AndAP>(ctx);
        for (const auto numeric_constraint : rule.get_body().get_numeric_constraints())
        {
            const auto constraint_cost = selector.get_constraint_cost(numeric_constraint, typename std::decay_t<decltype(AndAP::agg)> {});
            if (constraint_cost == std::numeric_limits<Cost>::max())
                return std::numeric_limits<Cost>::max();
            cost = AndAP::agg(cost, constraint_cost);
        }
        return cost;
    }
    else
    {
        auto selector = make_numeric_support_selector<AndAP>(ctx);
        for (const auto numeric_constraint : rule.get_body().get_numeric_constraints())
            if (selector.get_constraint_cost(numeric_constraint, MaxAggregation {}) == std::numeric_limits<Cost>::max())
                return std::numeric_limits<Cost>::max();
        return Cost(0);
    }
}

template<AndAnnotationPolicyConcept<GroundTag> AndAP,
         OrAnnotationPolicyConcept<GroundTag> OrAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP,
         typename EvaluateEffectExpression>
Cost aggregate_numeric_effect_support_cost(fd::GroundRuleView rule,
                                           const GroundCtx<OrAP, AndAP, TP, CP>& ctx,
                                           std::vector<NumericSelectionEntry>& selection,
                                           EvaluateEffectExpression&& evaluate_effect_expression)
{
    if constexpr (requires { AndAP::agg; })
    {
        auto cost = std::decay_t<decltype(AndAP::agg)>::identity();
        for (const auto literal : rule.get_body().template get_literals<f::FluentTag>())
        {
            if (!literal.get_polarity())
                continue;
            const auto* annotation = ctx.out().and_annot().find(literal.get_atom());
            assert(annotation && "enabled ground rule has a positive fluent body atom without a cost annotation");
            cost = AndAP::agg(cost, get_cost(*annotation));
        }

        auto selector = make_numeric_support_selector<AndAP>(ctx);
        selection.clear();
        auto constraint_selection = std::vector<NumericSelectionEntry> {};
        for (const auto numeric_constraint : rule.get_body().get_numeric_constraints())
        {
            const auto constraint_cost = selector.get_constraint_cost(numeric_constraint, constraint_selection, typename std::decay_t<decltype(AndAP::agg)> {});
            if (constraint_cost == std::numeric_limits<Cost>::max())
                return std::numeric_limits<Cost>::max();
            selection.insert(selection.end(), constraint_selection.begin(), constraint_selection.end());
            cost = AndAP::agg(cost, constraint_cost);
        }

        auto effect_selection = std::vector<NumericSelectionEntry> {};
        if (!evaluate_effect_expression(selector, effect_selection))
            return std::numeric_limits<Cost>::max();

        selection.insert(selection.end(), effect_selection.begin(), effect_selection.end());
        for (const auto& entry : effect_selection)
            cost = AndAP::agg(cost, entry.cost);
        return cost;
    }
    else
    {
        auto selector = make_numeric_support_selector<AndAP>(ctx);
        selection.clear();
        auto constraint_selection = std::vector<NumericSelectionEntry> {};
        for (const auto numeric_constraint : rule.get_body().get_numeric_constraints())
        {
            if (selector.get_constraint_cost(numeric_constraint, constraint_selection, MaxAggregation {}) == std::numeric_limits<Cost>::max())
                return std::numeric_limits<Cost>::max();
            selection.insert(selection.end(), constraint_selection.begin(), constraint_selection.end());
        }

        auto effect_selection = std::vector<NumericSelectionEntry> {};
        if (!evaluate_effect_expression(selector, effect_selection))
            return std::numeric_limits<Cost>::max();
        selection.insert(selection.end(), effect_selection.begin(), effect_selection.end());
        return Cost(0);
    }
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP,
         f::NumericEffectOpKind Op>
Cost aggregate_numeric_effect_rule_cost(fd::GroundRuleView rule,
                                        fd::GroundNumericEffectView<Op, f::FluentTag> effect,
                                        const GroundCtx<OrAP, AndAP, TP, CP>& ctx,
                                        std::vector<NumericSelectionEntry>& selection)
{
    const auto support_cost = aggregate_numeric_effect_support_cost<AndAP>(rule,
                                                                           ctx,
                                                                           selection,
                                                                           [&](const auto& selector, auto& selected)
                                                                           {
                                                                               if constexpr (!std::is_same_v<Op, f::Assign>)
                                                                                   if (empty(selector.select_fluent_interval(effect.get_fterm(), selected)))
                                                                                       return false;
                                                                               if (empty(selector.evaluate_effect_expression(effect.get_fexpr(), selected)))
                                                                                   return false;
                                                                               return true;
                                                                           });

    if (support_cost == std::numeric_limits<Cost>::max())
        return support_cost;

    auto metric_selection = std::vector<NumericSelectionEntry> {};
    const auto metric_cost = aggregate_metric_effect_cost(rule, ctx, metric_selection);
    if (metric_cost == std::numeric_limits<Cost>::max())
        return metric_cost;

    if constexpr (requires { AndAP::agg; })
        return aggregate_selection_cost(support_cost, metric_selection, AndAP::agg) + metric_cost;
    else
        return support_cost + metric_cost;
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
Cost aggregate_rule_cost(fd::GroundRuleView rule, const GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    return ygg::visit(
        [&](auto&& head) -> Cost
        {
            using Head = std::decay_t<decltype(head)>;
            if constexpr (std::is_same_v<Head, fd::GroundNumericEffectOperatorView<f::FluentTag>>)
            {
                auto best_cost = std::numeric_limits<Cost>::max();
                ygg::visit(
                    [&](auto&& effect)
                    {
                        auto selection = std::vector<NumericSelectionEntry> {};
                        best_cost = std::min(best_cost, aggregate_numeric_effect_rule_cost(rule, effect, ctx, selection));
                    },
                    head.get_variant());
                return best_cost;
            }
            else
            {
                if constexpr (requires { AndAP::agg; })
                {
                    const auto body_cost = aggregate_body_cost<AndAP>(rule, ctx);
                    if (body_cost == std::numeric_limits<Cost>::max())
                        return body_cost;
                    auto metric_selection = std::vector<NumericSelectionEntry> {};
                    const auto metric_cost = aggregate_metric_effect_cost(rule, ctx, metric_selection);
                    if (metric_cost == std::numeric_limits<Cost>::max())
                        return metric_cost;
                    return aggregate_selection_cost(body_cost, metric_selection, AndAP::agg) + metric_cost;
                }
                else
                {
                    auto selector = make_numeric_support_selector<AndAP>(ctx);
                    for (const auto numeric_constraint : rule.get_body().get_numeric_constraints())
                        if (selector.get_constraint_cost(numeric_constraint, MaxAggregation {}) == std::numeric_limits<Cost>::max())
                            return std::numeric_limits<Cost>::max();
                    return Cost(0);
                }
            }
        },
        rule.get_head());
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void push_rule(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundRuleView rule)
{
    auto& out = ctx.out();
    const auto rule_index = rule.get_index();
    if (at(out.rule_states(), rule_index).unsatisfied_count != 0 || at(out.rule_states(), rule_index).fired)
        return;

    const auto cost = aggregate_rule_cost(rule, ctx);
    if (cost == std::numeric_limits<Cost>::max())
        return;

    auto& queued_cost = at(out.rule_states(), rule_index).queued_cost;
    if (queued_cost && *queued_cost <= cost)
        return;
    queued_cost = cost;

    out.queue_storage().push_back(GroundQueueEntry { cost, out.queue().next_sequence++, rule });
    std::push_heap(out.queue_storage().begin(), out.queue_storage().end(), ygg::Greater<GroundQueueEntry> {});

    ++out.statistics().num_queue_pushes;
    out.statistics().max_queue_size = std::max(out.statistics().max_queue_size, static_cast<ygg::uint_t>(out.queue_storage().size()));
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void update_numeric_constraint_satisfaction(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundRuleView rule)
{
    auto& out = ctx.out();
    const auto rule_index = rule.get_index();
    auto& satisfied = at(out.rule_states(), rule_index).numeric_constraint_satisfied;
    const auto numeric_constraints = rule.get_body().get_numeric_constraints();
    if (satisfied.size() != numeric_constraints.size())
        satisfied.assign(numeric_constraints.size(), false);

    auto selector = make_numeric_support_selector<AndAP>(ctx);
    for (ygg::uint_t i = 0; i < numeric_constraints.size(); ++i)
    {
        if (satisfied[i])
            continue;

        if (selector.get_constraint_cost(numeric_constraints[i], MaxAggregation {}) == std::numeric_limits<Cost>::max())
            continue;

        satisfied[i] = true;
        auto& unsatisfied_count = at(out.rule_states(), rule_index).unsatisfied_count;
        assert(unsatisfied_count > 0);
        --unsatisfied_count;
    }
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void initialize_numeric_constraint_satisfaction(GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    for (const auto rule : ctx.in().program().get_ground_rules())
        update_numeric_constraint_satisfaction(ctx, rule);
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void seed_queue(GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    for (const auto rule : ctx.in().program().get_ground_rules())
        push_rule(ctx, rule);
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
std::optional<GroundQueueEntry> pop_next_entry(GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    auto& out = ctx.out();
    if (out.queue_storage().empty())
        return std::nullopt;

    std::pop_heap(out.queue_storage().begin(), out.queue_storage().end(), ygg::Greater<GroundQueueEntry> {});
    const auto entry = out.queue_storage().back();
    out.queue_storage().pop_back();
    ++out.statistics().num_queue_pops;
    return entry;
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool is_stale_entry(const GroundCtx<OrAP, AndAP, TP, CP>& ctx, const GroundQueueEntry& entry) noexcept
{
    const auto& out = ctx.out();
    const auto rule_index = entry.rule.get_index();
    return (has_atom_head(entry.rule) && at(out.rule_states(), rule_index).fired) || at(out.rule_states(), rule_index).unsatisfied_count != 0;
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void notify_fact_inserted(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundAtomView<f::FluentTag> fact)
{
    auto& out = ctx.out();
    const auto& dependencies = ctx.in().fluent_precondition_to_rules();
    const auto dependency_it = dependencies.find(fact);
    if (dependency_it == dependencies.end())
        return;

    for (const auto dependent_rule : dependency_it->second)
    {
        auto& unsatisfied_count = at(out.rule_states(), dependent_rule.get_index()).unsatisfied_count;
        if (unsatisfied_count == 0)
            continue;

        --unsatisfied_count;
        push_rule(ctx, dependent_rule);
    }
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void notify_fact_annotation_improved(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundAtomView<f::FluentTag> fact)
{
    auto& out = ctx.out();
    const auto& dependencies = ctx.in().fluent_precondition_to_rules();
    const auto dependency_it = dependencies.find(fact);
    if (dependency_it == dependencies.end())
        return;

    for (const auto dependent_rule : dependency_it->second)
        if (at(out.rule_states(), dependent_rule.get_index()).unsatisfied_count == 0)
            push_rule(ctx, dependent_rule);
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool derive_fact(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundAtomView<f::FluentTag> fact)
{
    auto& out = ctx.out();
    const auto inserted = out.fluent_atoms().insert(fact).second;
    if (inserted)
    {
        out.fluent_fact_sets().predicate.insert(fact);
        ++out.statistics().num_facts_derived;
    }
    return inserted;
}

template<typename Op>
ygg::ClosedInterval<ygg::float_t> apply_numeric_effect(Op, ygg::ClosedInterval<ygg::float_t> lhs, ygg::ClosedInterval<ygg::float_t> rhs)
{
    if constexpr (std::is_same_v<Op, f::Assign>)
        return rhs;
    else if constexpr (std::is_same_v<Op, f::Increase>)
        return lhs + rhs;
    else if constexpr (std::is_same_v<Op, f::Decrease>)
        return lhs - rhs;
    else if constexpr (std::is_same_v<Op, f::ScaleUp>)
        return lhs * rhs;
    else if constexpr (std::is_same_v<Op, f::ScaleDown>)
        return lhs / rhs;
    else
        static_assert(ygg::dependent_false<Op>::value, "Missing case");
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void notify_numeric_interval_changed(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundFunctionTermView<f::FluentTag> term)
{
    const auto dependency_it = ctx.in().fluent_function_term_to_rules().find(term);
    if (dependency_it == ctx.in().fluent_function_term_to_rules().end())
        return;

    for (const auto dependent_rule : dependency_it->second)
    {
        update_numeric_constraint_satisfaction(ctx, dependent_rule);
        if (at(ctx.out().rule_states(), dependent_rule.get_index()).unsatisfied_count == 0)
            push_rule(ctx, dependent_rule);
    }
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool derive_interval(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundFunctionTermView<f::FluentTag> term, ygg::ClosedInterval<ygg::float_t> interval)
{
    if (empty(interval))
        return false;

    auto& intervals = ctx.out().fluent_fterm_intervals();
    const auto it = intervals.find(term);
    if (it == intervals.end())
    {
        intervals.emplace(term, interval);
        ctx.out().fluent_fact_sets().function.insert(term, interval);
        return true;
    }

    const auto old_interval = it->second;
    it->second = hull(old_interval, interval);
    const auto changed = old_interval != it->second;
    if (changed)
        ctx.out().fluent_fact_sets().function.insert(term, interval);
    return changed;
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void update_numeric_annotation(GroundCtx<OrAP, AndAP, TP, CP>& ctx,
                               fd::GroundRuleView rule,
                               fd::GroundFunctionTermView<f::FluentTag> term,
                               ygg::ClosedInterval<ygg::float_t> interval,
                               Cost cost,
                               const std::vector<NumericSelectionEntry>& selection)
{
    if constexpr (requires { AndAP::agg; })
    {
        auto delta_annotation = GroundSelectedFunctionAnnotations {};
        auto metric = aggregate_body_metric<AndAP>(rule, ctx);
        metric = aggregate_numeric_selection_metric(metric, selection, ctx);
        auto metric_selection = std::vector<NumericSelectionEntry> {};
        metric = add_metric_delta<AndAP>(metric, aggregate_metric_effect_cost(rule, ctx, metric_selection));
        metric = aggregate_numeric_selection_metric(metric, metric_selection, ctx);
        auto numeric_supports = std::vector<NumericSupport<GroundTag>> {};
        append_numeric_supports(numeric_supports, selection);
        append_numeric_supports(numeric_supports, metric_selection);
        const auto context = GroundAndAnnotationContext { metric, cost, std::move(numeric_supports), rule, ctx.out().and_annot() };
        ctx.out().and_ap().update_annotation(term, interval, context, delta_annotation);

        if (const auto* annotation = delta_annotation.find(term, interval))
            ctx.out().numeric_and_annot().insert(term, interval, *annotation);
    }
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP,
         f::NumericEffectOpKind Op>
bool enqueue_numeric_effect(GroundCtx<OrAP, AndAP, TP, CP>& ctx,
                            fd::GroundRuleView rule,
                            fd::GroundNumericEffectView<Op, f::FluentTag> effect,
                            Cost effect_cost,
                            PendingNumericBuckets& pending_numeric)
{
    auto selector = make_numeric_support_selector<AndAP>(ctx);
    auto selection = std::vector<NumericSelectionEntry> {};
    const auto lhs = [&]
    {
        if constexpr (std::is_same_v<Op, f::Assign>)
            return ygg::ClosedInterval<ygg::float_t>();
        else
            return selector.select_fluent_interval(effect.get_fterm(), selection);
    }();
    if constexpr (!std::is_same_v<Op, f::Assign>)
        if (empty(lhs))
            return false;

    const auto rhs = selector.evaluate_effect_expression(effect.get_fexpr(), selection);
    if (empty(rhs))
        return false;

    const auto interval = apply_numeric_effect(Op {}, lhs, rhs);
    if (empty(interval))
        return false;

    const auto current = find_interval(ctx.out().facts(), effect.get_fterm());
    if (!empty(current) && subset(interval, current))
        return false;

    const auto used_transition_cost = ctx.out().cost_policy().get_cost(rule, effect.get_fterm(), interval);
    const auto transition_cost = used_transition_cost >= effect_cost ? Cost(0) : effect_cost - used_transition_cost;

    if (!pending_numeric.insert(transition_cost, effect.get_fterm(), interval))
        return false;

    update_numeric_annotation(ctx, rule, effect.get_fterm(), interval, transition_cost, selection);
    return true;
}

template<typename Head>
bool is_atom_head(const Head&) noexcept
{
    return std::is_same_v<Head, fd::GroundAtomView<f::FluentTag>>;
}

bool has_atom_head(fd::GroundRuleView rule) noexcept
{
    return ygg::visit([](auto&& head) { return is_atom_head(head); }, rule.get_head());
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
std::optional<GroundCostUpdate>
update_fact_annotation(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundRuleView rule, fd::GroundAtomView<f::FluentTag> fact, Cost cost)
{
    auto& out = ctx.out();
    auto delta_annotation = GroundSelectedPredicateAnnotations {};
    auto numeric_supports = std::vector<NumericSupport<GroundTag>> {};
    auto metric = aggregate_body_metric<AndAP>(rule, ctx);
    if constexpr (requires { AndAP::agg; })
    {
        auto selector = make_numeric_support_selector<AndAP>(ctx);
        auto selection = std::vector<NumericSelectionEntry> {};
        for (const auto numeric_constraint : rule.get_body().get_numeric_constraints())
            if (selector.get_constraint_cost(numeric_constraint, selection, typename std::decay_t<decltype(AndAP::agg)> {}) != std::numeric_limits<Cost>::max())
                append_numeric_supports(numeric_supports, selection);

        auto metric_selection = std::vector<NumericSelectionEntry> {};
        const auto metric_cost = aggregate_metric_effect_cost(rule, ctx, metric_selection);
        if (metric_cost != std::numeric_limits<Cost>::max())
        {
            metric = add_metric_delta<AndAP>(metric, metric_cost);
            metric = aggregate_numeric_selection_metric(metric, metric_selection, ctx);
            append_numeric_supports(numeric_supports, metric_selection);
        }
    }
    const auto context = GroundAndAnnotationContext { metric, cost, std::move(numeric_supports), rule, out.and_annot() };

    out.and_ap().record_achiever(fact, context);
    out.and_ap().update_annotation(fact, context, delta_annotation);

    if (const auto* annotation = delta_annotation.find(fact))
        return out.or_ap().update_annotation(fact, *annotation, out.and_annot());

    return std::nullopt;
}

bool is_annotation_improvement(const std::optional<GroundCostUpdate>& update) noexcept
{
    return update && (!update->old_cost || update->new_cost < *update->old_cost);
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool fire_rule(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundRuleView rule, Cost cost, PendingNumericBuckets& pending_numeric)
{
    auto& out = ctx.out();
    const auto rule_index = rule.get_index();
    if (has_atom_head(rule))
        at(out.rule_states(), rule_index).fired = true;
    ++out.statistics().num_rules_fired;

    auto stop = false;
    ygg::visit(
        [&](auto&& head)
        {
            using Head = std::decay_t<decltype(head)>;
            if constexpr (std::is_same_v<Head, fd::GroundAtomView<f::FluentTag>>)
            {
                const auto update = update_fact_annotation(ctx, rule, head, cost);
                const auto inserted = derive_fact(ctx, head);
                if (inserted)
                    notify_fact_inserted(ctx, head);
                if (is_annotation_improvement(update))
                    notify_fact_annotation_improved(ctx, head);
                stop = ctx.out().tp().check(ctx.in().program(), ctx.out().facts());
            }
            else if constexpr (std::is_same_v<Head, fd::GroundNumericEffectOperatorView<f::FluentTag>>)
            {
                ygg::visit([&](auto&& effect) { enqueue_numeric_effect(ctx, rule, effect, cost, pending_numeric); }, head.get_variant());
            }
        },
        rule.get_head());
    return stop;
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
Cost next_rule_cost(const GroundCtx<OrAP, AndAP, TP, CP>& ctx) noexcept
{
    return ctx.out().queue_storage().empty() ? std::numeric_limits<Cost>::max() : ctx.out().queue_storage().front().cost;
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool commit_numeric_bucket(GroundCtx<OrAP, AndAP, TP, CP>& ctx, PendingNumericBuckets& pending_numeric, Cost cost)
{
    auto bucket = pending_numeric.take(cost);
    auto changed_terms = std::vector<fd::GroundFunctionTermView<f::FluentTag>> {};
    changed_terms.reserve(bucket.size());

    for (const auto& [term, interval] : bucket)
        if (derive_interval(ctx, term, interval))
            changed_terms.push_back(term);

    for (const auto term : changed_terms)
        notify_numeric_interval_changed(ctx, term);

    return !changed_terms.empty() && ctx.out().tp().check(ctx.in().program(), ctx.out().facts());
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool process_rule_frontier(GroundCtx<OrAP, AndAP, TP, CP>& ctx, PendingNumericBuckets& pending_numeric, Cost cost)
{
    while (next_rule_cost(ctx) == cost)
    {
        auto entry = pop_next_entry(ctx);
        assert(entry);

        auto& out = ctx.out();
        const auto rule_index = entry->rule.get_index();
        auto& queued_cost = at(out.rule_states(), rule_index).queued_cost;
        if (!queued_cost || *queued_cost != entry->cost)
        {
            ++out.statistics().num_stale_queue_pops;
            continue;
        }
        queued_cost.reset();

        if (is_stale_entry(ctx, *entry))
        {
            ++out.statistics().num_stale_queue_pops;
            continue;
        }

        if (fire_rule(ctx, entry->rule, entry->cost, pending_numeric))
            return true;

        if (pending_numeric.min_cost() == cost)
            return false;
    }

    return false;
}

}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void solve_ground_queue(ProgramExecutionContext<GroundTag, OrAP, AndAP, TP, CP>& ctx)
{
    initialize_numeric_constraint_satisfaction(ctx);
    seed_queue(ctx);

    auto pending_numeric = PendingNumericBuckets {};
    while (next_rule_cost(ctx) != std::numeric_limits<Cost>::max() || !pending_numeric.is_empty())
    {
        const auto rule_cost = next_rule_cost(ctx);
        const auto numeric_cost = pending_numeric.min_cost();
        const auto cost = std::min(rule_cost, numeric_cost);

        if (numeric_cost == cost && commit_numeric_bucket(ctx, pending_numeric, cost))
            break;

        if (rule_cost == cost && process_rule_frontier(ctx, pending_numeric, cost))
            break;
    }
}

void solve_ground_queue(ProgramExecutionContext<GroundTag>& ctx)
{
    solve_ground_queue<NoOrAnnotationPolicy<GroundTag>, NoAndAnnotationPolicy<GroundTag>, NoTerminationPolicy<GroundTag>, RuleCostPolicy<GroundTag>>(ctx);
}

template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         NoOrAnnotationPolicy<GroundTag>,
                                                         NoAndAnnotationPolicy<GroundTag>,
                                                         NoTerminationPolicy<GroundTag>,
                                                         RuleCostPolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AndAnnotationPolicy<GroundTag, SumAggregation>,
                                                         NoTerminationPolicy<GroundTag>,
                                                         RuleCostPolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AndAnnotationPolicy<GroundTag, SumAggregation>,
                                                         TerminationPolicy<GroundTag, SumAggregation>,
                                                         RuleCostPolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                         NoTerminationPolicy<GroundTag>,
                                                         RuleCostPolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                         TerminationPolicy<GroundTag, MaxAggregation>,
                                                         RuleCostPolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AchieverAndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                         TerminationPolicy<GroundTag, MaxAggregation>,
                                                         RuleCostPolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AndAnnotationPolicy<GroundTag, SumAggregation>,
                                                         NoTerminationPolicy<GroundTag>,
                                                         RuleCostOverridePolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AndAnnotationPolicy<GroundTag, SumAggregation>,
                                                         TerminationPolicy<GroundTag, SumAggregation>,
                                                         RuleCostOverridePolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                         NoTerminationPolicy<GroundTag>,
                                                         RuleCostOverridePolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                         TerminationPolicy<GroundTag, MaxAggregation>,
                                                         RuleCostOverridePolicy<GroundTag>>& ctx);
template void solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                         OrAnnotationPolicy<GroundTag>,
                                                         AchieverAndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                         TerminationPolicy<GroundTag, MaxAggregation>,
                                                         RuleCostOverridePolicy<GroundTag>>& ctx);
}
