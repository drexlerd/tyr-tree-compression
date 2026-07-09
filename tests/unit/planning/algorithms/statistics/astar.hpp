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

#include "statistics.hpp"

#include <fmt/core.h>

namespace p = tyr::planning;

namespace tyr::tests
{
namespace
{
class TppExpandedNodeTraceEventHandler : public p::astar_eager::EventHandlerBase<TppExpandedNodeTraceEventHandler, StatisticsTaskKind>
{
private:
    using Base = p::astar_eager::EventHandlerBase<TppExpandedNodeTraceEventHandler, StatisticsTaskKind>;
    friend Base;

    p::Heuristic<StatisticsTaskKind>& m_heuristic;

    void on_expand_node_impl(const p::Node<StatisticsTaskKind>& node) const { static_cast<void>(node); }
    void on_expand_goal_node_impl(const p::Node<StatisticsTaskKind>& node) const { static_cast<void>(node); }
    void on_generate_node_impl(const p::LabeledNode<StatisticsTaskKind>& labeled_succ_node) const { static_cast<void>(labeled_succ_node); }
    void on_generate_node_relaxed_impl(const p::LabeledNode<StatisticsTaskKind>& labeled_succ_node) const { static_cast<void>(labeled_succ_node); }
    void on_generate_node_not_relaxed_impl(const p::LabeledNode<StatisticsTaskKind>& labeled_succ_node) const { static_cast<void>(labeled_succ_node); }
    void on_close_node_impl(const p::Node<StatisticsTaskKind>& node) const { static_cast<void>(node); }
    void on_prune_node_impl(const p::Node<StatisticsTaskKind>& node) const { static_cast<void>(node); }
    void on_start_search_impl(const p::Node<StatisticsTaskKind>& node, ygg::float_t f_value) const
    {
        static_cast<void>(node);
        static_cast<void>(f_value);
    }
    void on_finish_f_layer_impl(ygg::float_t f_value, uint64_t num_expanded_states, uint64_t num_generated_states) const
    {
        static_cast<void>(f_value);
        static_cast<void>(num_expanded_states);
        static_cast<void>(num_generated_states);
    }
    void on_end_search_impl(p::SearchStatus status) const { static_cast<void>(status); }
    void on_solved_impl(const p::Plan<StatisticsTaskKind>& plan) const { static_cast<void>(plan); }

public:
    explicit TppExpandedNodeTraceEventHandler(p::Heuristic<StatisticsTaskKind>& heuristic) : Base(0), m_heuristic(heuristic) {}

    void on_expand_node(const p::Node<StatisticsTaskKind>& node) override
    {
        Base::on_expand_node(node);
        print_node("Expanded", this->get_statistics().get_num_expanded(), node);
    }

    void on_generate_node(const p::Node<StatisticsTaskKind>& source_node, const p::LabeledNode<StatisticsTaskKind>& labeled_succ_node) override
    {
        Base::on_generate_node(source_node, labeled_succ_node);
        print_node("Generated", this->get_statistics().get_num_generated(), labeled_succ_node.node);
    }

    void print_node(const char* event, uint64_t count, const p::Node<StatisticsTaskKind>& node)
    {
        const auto g = ygg::FloatTolerance<ygg::float_t>::canonicalize(node.get_metric());
        const auto h = ygg::FloatTolerance<ygg::float_t>::canonicalize(m_heuristic.evaluate(node));
        const auto f = ygg::FloatTolerance<ygg::float_t>::canonicalize(g + h);
        fmt::print("[ASTAR][TPP] {} #{} state={} g={} h={} f={}\n", event, count, node.get_state().get_index().get_value(), g, h, f);
    }
};

p::astar_eager::EventHandlerPtr<StatisticsTaskKind>
create_astar_event_handler(const SearchCase& test_case, const std::string& key, p::Heuristic<StatisticsTaskKind>& heuristic)
{
    if (test_case.name == "Tpp" && key == "hff_unit")
        return std::make_shared<TppExpandedNodeTraceEventHandler>(heuristic);

    return p::astar_eager::DefaultEventHandler<StatisticsTaskKind>::create();
}

void check_statistics(const SearchStatistics& expected,
                      const SearchCase& test_case,
                      const std::string& key,
                      const std::string& heuristic_name,
                      p::CostMode cost_mode)
{
    auto context = create_search_context<StatisticsTaskKind>(test_case.domain_file, test_case.task_file);
    auto heuristic = create_search_heuristic<StatisticsTaskKind>(heuristic_name, context, cost_mode);
    auto event_handler = create_astar_event_handler(test_case, key, *heuristic);

    auto options = p::astar_eager::Options<StatisticsTaskKind>();
    options.event_handler = event_handler;
    options.action_cost_mode = cost_mode;
    const auto result = p::astar_eager::find_solution(*context.task, *context.successor_generator, *heuristic, options);

    expect_statistics(expected, event_handler->get_statistics(), result);
}
}

class AstarStatisticsTest : public ::testing::TestWithParam<SearchCase>
{
};

TEST_P(AstarStatisticsTest, StatisticsMatchFixture)
{
    const auto& test_case = GetParam();

    for (const auto& [key, expected] : test_case.configs)
    {
        SCOPED_TRACE(key);
        const auto [heuristic_name, cost_mode] = parse_costed_heuristic_key(key);
        check_statistics(expected, test_case, key, heuristic_name, cost_mode);
    }
}

INSTANTIATE_TEST_SUITE_P(TyrPlanningAstarStatistics,
                         AstarStatisticsTest,
                         ::testing::ValuesIn(load_search_cases(kStatisticsFixture)),
                         [](const testing::TestParamInfo<SearchCase>& info) { return info.param.name; });

}
