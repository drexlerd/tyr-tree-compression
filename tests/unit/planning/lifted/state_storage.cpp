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

#include "tyr/planning/lifted/state_storage.hpp"

#include "tyr/planning/state_storage/config.hpp"

#if defined(TYR_STATE_STORAGE_HASHSET)
#include "tyr/planning/lifted/state_storage/hash_set/fact.hpp"
#elif defined(TYR_STATE_STORAGE_TREE)
#include "tyr/planning/lifted/state_storage/tree_compression/fact.hpp"
#endif

#include <gtest/gtest.h>
#include <iterator>
#include <vector>
#include <yggdrasil/containers/dynamic_bitset.hpp>

namespace p = tyr::planning;

TEST(TyrTests, TyrPlanningLiftedStateStorageCombinesFactsAndAtoms)
{
    auto context = p::StateStorageContext<p::LiftedTag, p::StateStoragePolicyTag> {};
    auto backend = p::FactStorageBackend<p::LiftedTag, p::StateStoragePolicyTag>(context);
    auto facts = p::FactUnpackedStorage<p::LiftedTag> {};
    auto atoms = p::AtomUnpackedStorage<p::LiftedTag> {};

    for (const auto i : { 0U, 2U, 5U })
        ygg::set(i, true, facts.indices);
    for (const auto i : { 0U, 1U, 5U })
        ygg::set(i, true, atoms.indices);

    const auto packed = backend.insert(facts, atoms);
    auto encoded = std::vector<ygg::uint_t> {};
#if defined(TYR_STATE_STORAGE_HASHSET)
    const auto view = context.uint_vec_set[packed.index];
    encoded.assign(view.begin(), view.end());
#elif defined(TYR_STATE_STORAGE_TREE)
    valla::read_sequence(packed.slot, context.uint_nodes, std::back_inserter(encoded));
#endif
    EXPECT_EQ(encoded, (std::vector<ygg::uint_t> { 0, 1, 3, 4, 10, 11 }));

    auto unpacked_facts = p::FactUnpackedStorage<p::LiftedTag> {};
    auto unpacked_atoms = p::AtomUnpackedStorage<p::LiftedTag> {};
    ygg::set(7, true, unpacked_facts.indices);
    ygg::set(8, true, unpacked_atoms.indices);
    backend.unpack(packed, unpacked_facts, unpacked_atoms);
    EXPECT_EQ(unpacked_facts.indices, facts.indices);
    EXPECT_EQ(unpacked_atoms.indices, atoms.indices);

    const auto empty = backend.insert({}, {});
    backend.unpack(empty, unpacked_facts, unpacked_atoms);
    EXPECT_TRUE(unpacked_facts.indices.empty());
    EXPECT_TRUE(unpacked_atoms.indices.empty());
}
