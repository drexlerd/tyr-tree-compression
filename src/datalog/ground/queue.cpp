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
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include <yggdrasil/containers/variant.hpp>
#include <yggdrasil/semantics/comparators.hpp>

namespace tyr::datalog::experimental
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

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void push_rule(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundRuleView rule)
{
    auto& out = ctx.out();
    out.queue_storage().push_back(GroundQueueEntry { at(out.unsatisfied_counts(), rule.get_index()), rule });
    std::push_heap(out.queue_storage().begin(), out.queue_storage().end(), ygg::Greater<GroundQueueEntry> {});

    ++out.statistics().num_queue_pushes;
    out.statistics().max_queue_size = std::max(out.statistics().max_queue_size, static_cast<ygg::uint_t>(out.queue_storage().size()));
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
    return at(out.fired_rules(), rule_index) || entry.unsatisfied_count != at(out.unsatisfied_counts(), rule_index);
}

template<AndAnnotationPolicyConcept<GroundTag> AndAP>
Cost aggregate_body_cost(fd::GroundRuleView rule, const GroundSelectedPredicateAnnotations& annotations) noexcept
{
    if constexpr (requires { AndAP::agg; })
    {
        auto cost = std::decay_t<decltype(AndAP::agg)>::identity();
        for (const auto literal : rule.get_body().template get_literals<f::FluentTag>())
        {
            if (!literal.get_polarity())
                continue;
            if (const auto* annotation = annotations.find(literal.get_atom()))
                cost = AndAP::agg(cost, get_cost(*annotation));
        }
        return cost;
    }
    else
    {
        return Cost(0);
    }
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void notify_rules_waiting_on(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundAtomView<f::FluentTag> fact)
{
    auto& out = ctx.out();
    const auto& dependencies = ctx.in().fluent_precondition_to_rules();
    const auto dependency_it = dependencies.find(fact);
    if (dependency_it == dependencies.end())
        return;

    for (const auto dependent_rule : dependency_it->second)
    {
        if (at(out.fired_rules(), dependent_rule.get_index()) || at(out.unsatisfied_counts(), dependent_rule.get_index()) == 0)
            continue;

        --at(out.unsatisfied_counts(), dependent_rule.get_index());
        push_rule(ctx, dependent_rule);
    }
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool derive_fact(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundAtomView<f::FluentTag> fact)
{
    auto& out = ctx.out();
    const auto inserted = out.fluent_atoms().insert(fact).second;
    if (!inserted)
        return false;

    ++out.statistics().num_facts_derived;
    notify_rules_waiting_on(ctx, fact);
    return true;
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
void update_fact_annotation(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundRuleView rule, fd::GroundAtomView<f::FluentTag> fact)
{
    auto& out = ctx.out();
    auto delta_annotation = GroundSelectedPredicateAnnotations {};
    const auto context =
        GroundAndAnnotationContext { aggregate_body_cost<AndAP>(rule, out.and_annot()), rule, out.cost_policy().get_cost(rule), out.and_annot() };

    out.and_ap().record_achiever(fact, context);
    out.and_ap().update_annotation(fact, context, delta_annotation);

    if (const auto* annotation = delta_annotation.find(fact))
        out.or_ap().update_annotation(fact, *annotation, out.and_annot());
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool should_stop(const GroundCtx<OrAP, AndAP, TP, CP>& ctx) noexcept
{
    return ctx.out().tp().check(ctx.in().program(), ctx.out().facts());
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
bool fire_rule(GroundCtx<OrAP, AndAP, TP, CP>& ctx, fd::GroundRuleView rule)
{
    auto& out = ctx.out();
    at(out.fired_rules(), rule.get_index()) = true;
    ++out.statistics().num_rules_fired;

    auto stop = false;
    ygg::visit(
        [&](auto&& head)
        {
            using Head = std::decay_t<decltype(head)>;
            if constexpr (std::is_same_v<Head, fd::GroundAtomView<f::FluentTag>>)
            {
                update_fact_annotation(ctx, rule, head);
                derive_fact(ctx, head);
                stop = should_stop(ctx);
            }
        },
        rule.get_head());
    return stop;
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
GroundQueueResult make_result(const GroundCtx<OrAP, AndAP, TP, CP>& ctx)
{
    const auto& out = ctx.out();
    auto fluent_atoms = std::vector<fd::GroundAtomView<f::FluentTag>>(out.fluent_atoms().begin(), out.fluent_atoms().end());
    return GroundQueueResult { std::move(fluent_atoms), out.statistics() };
}
}

template<OrAnnotationPolicyConcept<GroundTag> OrAP,
         AndAnnotationPolicyConcept<GroundTag> AndAP,
         TerminationPolicyConcept<GroundTag> TP,
         RuleCostPolicyConcept<GroundTag> CP>
GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag, OrAP, AndAP, TP, CP>& ctx)
{
    seed_queue(ctx);

    while (auto entry = pop_next_entry(ctx))
    {
        auto& out = ctx.out();
        if (is_stale_entry(ctx, *entry))
        {
            ++out.statistics().num_stale_queue_pops;
            continue;
        }

        if (entry->unsatisfied_count != 0)
            break;

        if (fire_rule(ctx, entry->rule))
            break;
    }

    return make_result(ctx);
}

GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag>& ctx)
{
    return solve_ground_queue<NoOrAnnotationPolicy<GroundTag>, NoAndAnnotationPolicy<GroundTag>, NoTerminationPolicy<GroundTag>, RuleCostPolicy<GroundTag>>(
        ctx);
}

template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      NoOrAnnotationPolicy<GroundTag>,
                                                                      NoAndAnnotationPolicy<GroundTag>,
                                                                      NoTerminationPolicy<GroundTag>,
                                                                      RuleCostPolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AndAnnotationPolicy<GroundTag, SumAggregation>,
                                                                      NoTerminationPolicy<GroundTag>,
                                                                      RuleCostPolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AndAnnotationPolicy<GroundTag, SumAggregation>,
                                                                      TerminationPolicy<GroundTag, SumAggregation>,
                                                                      RuleCostPolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                                      NoTerminationPolicy<GroundTag>,
                                                                      RuleCostPolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                                      TerminationPolicy<GroundTag, MaxAggregation>,
                                                                      RuleCostPolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AchieverAndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                                      TerminationPolicy<GroundTag, MaxAggregation>,
                                                                      RuleCostPolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AndAnnotationPolicy<GroundTag, SumAggregation>,
                                                                      NoTerminationPolicy<GroundTag>,
                                                                      RuleCostOverridePolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AndAnnotationPolicy<GroundTag, SumAggregation>,
                                                                      TerminationPolicy<GroundTag, SumAggregation>,
                                                                      RuleCostOverridePolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                                      NoTerminationPolicy<GroundTag>,
                                                                      RuleCostOverridePolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                                      TerminationPolicy<GroundTag, MaxAggregation>,
                                                                      RuleCostOverridePolicy<GroundTag>>& ctx);
template GroundQueueResult solve_ground_queue(ProgramExecutionContext<GroundTag,
                                                                      OrAnnotationPolicy<GroundTag>,
                                                                      AchieverAndAnnotationPolicy<GroundTag, MaxAggregation>,
                                                                      TerminationPolicy<GroundTag, MaxAggregation>,
                                                                      RuleCostOverridePolicy<GroundTag>>& ctx);

}
