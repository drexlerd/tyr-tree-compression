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

#include "tyr/planning/lifted/state_storage/tree_compression/fact.hpp"

#include "tyr/planning/lifted/state_storage/tree_compression/context.hpp"
#include "tyr/planning/lifted/task.hpp"

#include <algorithm>
#include <cassert>
#include <limits>

namespace tyr::planning
{
namespace
{
ygg::uint_t encode_index(size_t index, bool is_derived)
{
    assert(index <= (std::numeric_limits<ygg::uint_t>::max() >> 1));
    return (static_cast<ygg::uint_t>(index) << 1) | static_cast<ygg::uint_t>(is_derived);
}
}

FactStorageBackend<LiftedTag, TreeCompression>::FactStorageBackend(StateStorageContext<LiftedTag, TreeCompression>& ctx) : m_uint_nodes(ctx.uint_nodes) {}

typename FactStorageBackend<LiftedTag, TreeCompression>::Packed FactStorageBackend<LiftedTag, TreeCompression>::insert(const FactUnpacked& facts,
                                                                                                                       const AtomUnpacked& atoms)
{
    m_uint_node_buffer.clear();
    for (auto i = facts.indices.find_first(); i != boost::dynamic_bitset<>::npos; i = facts.indices.find_next(i))
        m_uint_node_buffer.push_back(encode_index(i, false));
    for (auto i = atoms.indices.find_first(); i != boost::dynamic_bitset<>::npos; i = atoms.indices.find_next(i))
        m_uint_node_buffer.push_back(encode_index(i, true));
    std::ranges::sort(m_uint_node_buffer);

    const auto slot = valla::insert_sequence(m_uint_node_buffer, m_uint_nodes);
    return FactStorageBackend<LiftedTag, TreeCompression>::Packed { slot };
}

void FactStorageBackend<LiftedTag, TreeCompression>::unpack(const typename FactStorageBackend<LiftedTag, TreeCompression>::Packed& packed,
                                                            FactUnpacked& facts,
                                                            AtomUnpacked& atoms)
{
    m_uint_node_buffer.clear();
    facts.indices.clear();
    atoms.indices.clear();

    valla::read_sequence(packed.slot, m_uint_nodes, std::back_inserter(m_uint_node_buffer));

    for (const auto encoded : m_uint_node_buffer)
    {
        if (encoded & 1)
            ygg::set(encoded >> 1, true, atoms.indices);
        else
            ygg::set(encoded >> 1, true, facts.indices);
    }
}

}
