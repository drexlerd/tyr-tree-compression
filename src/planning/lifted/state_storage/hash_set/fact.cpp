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

#include "tyr/planning/lifted/state_storage/hash_set/fact.hpp"

#include "tyr/planning/lifted/state_storage/hash_set/context.hpp"
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

FactStorageBackend<LiftedTag, HashSet>::FactStorageBackend(StateStorageContext<LiftedTag, HashSet>& ctx) : m_uint_vec_set(ctx.uint_vec_set), m_buffer() {}

typename FactStorageBackend<LiftedTag, HashSet>::Packed FactStorageBackend<LiftedTag, HashSet>::insert(const FactUnpacked& facts, const AtomUnpacked& atoms)
{
    m_buffer.clear();
    for (auto i = facts.indices.find_first(); i != boost::dynamic_bitset<>::npos; i = facts.indices.find_next(i))
        m_buffer.push_back(encode_index(i, false));
    for (auto i = atoms.indices.find_first(); i != boost::dynamic_bitset<>::npos; i = atoms.indices.find_next(i))
        m_buffer.push_back(encode_index(i, true));
    std::ranges::sort(m_buffer);

    return FactStorageBackend<LiftedTag, HashSet>::Packed { m_uint_vec_set.insert(m_buffer) };
}

void FactStorageBackend<LiftedTag, HashSet>::unpack(const typename FactStorageBackend<LiftedTag, HashSet>::Packed& packed,
                                                    FactUnpacked& facts,
                                                    AtomUnpacked& atoms)
{
    const auto view = m_uint_vec_set[packed.index];

    facts.indices.clear();
    atoms.indices.clear();
    for (const auto encoded : view)
    {
        if (encoded & 1)
            ygg::set(encoded >> 1, true, atoms.indices);
        else
            ygg::set(encoded >> 1, true, facts.indices);
    }
}

}
