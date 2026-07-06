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

#include "tyr/datalog/lifted/policies/numeric_support.hpp"

namespace fd = tyr::formalism::datalog;
namespace f = tyr::formalism;

namespace tyr::datalog
{

void NumericSupportSelectorWorkspace::clear() noexcept { selection.clear(); }

NumericSupportSelector::NumericSupportSelector(const FactSets& fact_sets, const NumericIntervalAnnotations<LiftedTag>& annotations) :
    m_fact_sets(fact_sets),
    m_annotations(annotations)
{
}

ygg::ClosedInterval<ygg::float_t> NumericSupportSelector::lookup_static(fd::GroundFunctionTermView<f::StaticTag> term) const
{
    return m_fact_sets.get<f::StaticTag>().function[term];
}

ygg::ClosedInterval<ygg::float_t> NumericSupportSelector::current_interval(Key key) const { return m_fact_sets.get<f::FluentTag>().function[key]; }

const NumericIntervalAnnotations<LiftedTag>::Entries* NumericSupportSelector::find_entries(Key key) const
{
    const auto relation_it = m_annotations.partitions().find(key.get_relation());
    if (relation_it == m_annotations.partitions().end())
        return nullptr;

    const auto row_it = relation_it->second.find(key.get_index().row);
    return row_it == relation_it->second.end() ? nullptr : &row_it->second;
}

}
