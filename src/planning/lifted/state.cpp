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

#include "tyr/planning/lifted/state_builder.hpp"
#include "tyr/planning/lifted/state_repository.hpp"
#include "tyr/planning/lifted/state_view.hpp"
#include "tyr/planning/lifted/task.hpp"

#include <cassert>
#include <limits>

namespace tyr
{
using namespace planning;

ygg::Index<State<LiftedTag>> UnpackedState<LiftedTag>::get_index() const { return m_index; }

void UnpackedState<LiftedTag>::set(ygg::Index<State<LiftedTag>> index) { m_index = index; }

::tyr::formalism::planning::FDRValue UnpackedState<LiftedTag>::get(ygg::Index<::tyr::formalism::planning::FDRVariable<::tyr::formalism::FluentTag>> index) const
{
    return ::tyr::formalism::planning::FDRValue(ygg::test(ygg::uint_t(index), m_fact_storage.indices));
}

void UnpackedState<LiftedTag>::set(ygg::Data<::tyr::formalism::planning::FDRFact<::tyr::formalism::FluentTag>> fact)
{
    assert(ygg::uint_t(fact.value) < 2);  // can only handle binary using bitsets
    ygg::set(ygg::uint_t(fact.variable), bool(ygg::uint_t(fact.value)), m_fact_storage.indices);
}

ygg::float_t UnpackedState<LiftedTag>::get(ygg::Index<::tyr::formalism::planning::GroundFunctionTerm<::tyr::formalism::FluentTag>> index) const
{
    return ygg::get(ygg::uint_t(index), m_numeric_storage.values, std::numeric_limits<ygg::float_t>::quiet_NaN());
}

void UnpackedState<LiftedTag>::set(ygg::Index<::tyr::formalism::planning::GroundFunctionTerm<::tyr::formalism::FluentTag>> index, ygg::float_t value)
{
    ygg::set(ygg::uint_t(index), value, m_numeric_storage.values, std::numeric_limits<ygg::float_t>::quiet_NaN());
}

bool UnpackedState<LiftedTag>::test(ygg::Index<::tyr::formalism::planning::GroundAtom<::tyr::formalism::DerivedTag>> index) const
{
    return ygg::test(ygg::uint_t(index), m_atom_storage.indices);
}

void UnpackedState<LiftedTag>::set(ygg::Index<::tyr::formalism::planning::GroundAtom<::tyr::formalism::DerivedTag>> index)
{
    ygg::set(ygg::uint_t(index), true, m_atom_storage.indices);
}

::tyr::formalism::planning::FDRValue UnpackedState<LiftedTag>::get(::tyr::formalism::planning::FDRVariableView<::tyr::formalism::FluentTag> view) const
{
    return get(view.get_index());
}

void UnpackedState<LiftedTag>::set(::tyr::formalism::planning::FDRFactView<::tyr::formalism::FluentTag> view) { set(view.get_data()); }

ygg::float_t UnpackedState<LiftedTag>::get(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::FluentTag> view) const
{
    return get(view.get_index());
}

void UnpackedState<LiftedTag>::set(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::FluentTag> view, ygg::float_t value)
{
    set(view.get_index(), value);
}

bool UnpackedState<LiftedTag>::test(::tyr::formalism::planning::GroundAtomView<::tyr::formalism::DerivedTag> view) const { return test(view.get_index()); }

void UnpackedState<LiftedTag>::set(::tyr::formalism::planning::GroundAtomView<::tyr::formalism::DerivedTag> view) { set(view.get_index()); }

void UnpackedState<LiftedTag>::clear()
{
    clear_unextended_part();
    clear_extended_part();
}

void UnpackedState<LiftedTag>::clear_unextended_part()
{
    m_fact_storage.indices.clear();
    m_numeric_storage.values.clear();
}

void UnpackedState<LiftedTag>::clear_extended_part() { m_atom_storage.indices.clear(); }

void UnpackedState<LiftedTag>::assign_unextended_part(const UnpackedState<LiftedTag>& other)
{
    m_fact_storage = other.m_fact_storage;
    m_numeric_storage = other.m_numeric_storage;
}

FDRFactRange<LiftedTag, ::tyr::formalism::FluentTag> UnpackedState<LiftedTag>::get_fluent_facts() const noexcept
{
    return FDRFactRange<LiftedTag, ::tyr::formalism::FluentTag>(m_fact_storage.indices);
}

AtomRange<::tyr::formalism::DerivedTag> UnpackedState<LiftedTag>::get_derived_atoms() const noexcept
{
    return AtomRange<::tyr::formalism::DerivedTag>(m_atom_storage.indices);
}

FunctionTermValueRange<::tyr::formalism::FluentTag> UnpackedState<LiftedTag>::get_fluent_fterm_values() const noexcept
{
    return FunctionTermValueRange<::tyr::formalism::FluentTag>(m_numeric_storage.values);
}

NumericUnpackedStorage<LiftedTag>& UnpackedState<LiftedTag>::get_numeric_variables() noexcept { return m_numeric_storage; }

const NumericUnpackedStorage<LiftedTag>& UnpackedState<LiftedTag>::get_numeric_variables() const noexcept { return m_numeric_storage; }

template<::tyr::formalism::FactKind T>
LiftedUnpackedAtomStorage<T>& UnpackedState<LiftedTag>::get_atoms() noexcept
{
    if constexpr (std::same_as<T, ::tyr::formalism::FluentTag>)
        return m_fact_storage;
    else if constexpr (std::same_as<T, ::tyr::formalism::DerivedTag>)
        return m_atom_storage;
}

template<::tyr::formalism::FactKind T>
const LiftedUnpackedAtomStorage<T>& UnpackedState<LiftedTag>::get_atoms() const noexcept
{
    if constexpr (std::same_as<T, ::tyr::formalism::FluentTag>)
        return m_fact_storage;
    else if constexpr (std::same_as<T, ::tyr::formalism::DerivedTag>)
        return m_atom_storage;
}

template FactUnpackedStorage<LiftedTag>& UnpackedState<LiftedTag>::get_atoms<::tyr::formalism::FluentTag>() noexcept;
template AtomUnpackedStorage<LiftedTag>& UnpackedState<LiftedTag>::get_atoms<::tyr::formalism::DerivedTag>() noexcept;
template const FactUnpackedStorage<LiftedTag>& UnpackedState<LiftedTag>::get_atoms<::tyr::formalism::FluentTag>() const noexcept;
template const AtomUnpackedStorage<LiftedTag>& UnpackedState<LiftedTag>::get_atoms<::tyr::formalism::DerivedTag>() const noexcept;

}

namespace ygg
{
using namespace ::tyr;
namespace planning = ::tyr::planning;

LiftedStateView::View(std::shared_ptr<planning::StateRepository<planning::LiftedTag>> owner,
                      ygg::SharedObjectPoolPtr<planning::UnpackedState<planning::LiftedTag>> unpacked) noexcept :
    m_state_repository(std::move(owner)),
    m_unpacked(std::move(unpacked))
{
}

LiftedStateView::~View() = default;

LiftedStateView::View(const View&) = default;

LiftedStateView::View(View&&) noexcept = default;

LiftedStateView& LiftedStateView::operator=(const View&) = default;

LiftedStateView& LiftedStateView::operator=(View&&) noexcept = default;

ygg::Index<planning::State<planning::LiftedTag>> LiftedStateView::get_index() const { return m_unpacked->get_index(); }

std::tuple<ygg::Index<planning::State<planning::LiftedTag>>, ygg::uint_t> LiftedStateView::identifying_members() const noexcept
{
    return std::make_tuple(get_index(), m_state_repository->get_index());
}

::tyr::formalism::planning::FDRValue LiftedStateView::get(ygg::Index<::tyr::formalism::planning::FDRVariable<::tyr::formalism::FluentTag>> index) const
{
    return m_unpacked->get(index);
}

ygg::float_t LiftedStateView::get(ygg::Index<::tyr::formalism::planning::GroundFunctionTerm<::tyr::formalism::FluentTag>> index) const
{
    return m_unpacked->get(index);
}

bool LiftedStateView::test(ygg::Index<::tyr::formalism::planning::GroundAtom<::tyr::formalism::DerivedTag>> index) const { return m_unpacked->test(index); }

const std::shared_ptr<planning::StateRepository<planning::LiftedTag>>& LiftedStateView::get_state_repository() const noexcept { return m_state_repository; }

const planning::UnpackedState<planning::LiftedTag>& LiftedStateView::get_unpacked_state() const noexcept { return *m_unpacked; }

bool LiftedStateView::test(::tyr::formalism::planning::GroundAtomView<::tyr::formalism::StaticTag> view) const { return test(view.get_index()); }

ygg::float_t LiftedStateView::get(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::StaticTag> view) const { return get(view.get_index()); }

::tyr::formalism::planning::FDRValue LiftedStateView::get(::tyr::formalism::planning::FDRVariableView<::tyr::formalism::FluentTag> view) const
{
    return get(view.get_index());
}

ygg::float_t LiftedStateView::get(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::FluentTag> view) const { return get(view.get_index()); }

bool LiftedStateView::test(::tyr::formalism::planning::GroundAtomView<::tyr::formalism::DerivedTag> view) const { return test(view.get_index()); }

bool LiftedStateView::test(ygg::Index<::tyr::formalism::planning::GroundAtom<::tyr::formalism::StaticTag>> index) const
{
    return m_state_repository->get_task()->test(index);
}

ygg::float_t LiftedStateView::get(ygg::Index<::tyr::formalism::planning::GroundFunctionTerm<::tyr::formalism::StaticTag>> index) const
{
    return m_state_repository->get_task()->get(index);
}

template<::tyr::formalism::FactKind T>
const boost::dynamic_bitset<>& LiftedStateView::get_atoms() const noexcept
{
    if constexpr (std::is_same_v<T, ::tyr::formalism::StaticTag>)
        return m_state_repository->get_task()->get_static_atoms_bitset();
    else if constexpr (std::is_same_v<T, ::tyr::formalism::FluentTag> || std::is_same_v<T, ::tyr::formalism::DerivedTag>)
        return m_unpacked->template get_atoms<T>().indices;
    else
        static_assert(ygg::dependent_false<T>::value, "Missing case");
}

template const boost::dynamic_bitset<>& LiftedStateView::get_atoms<::tyr::formalism::StaticTag>() const noexcept;
template const boost::dynamic_bitset<>& LiftedStateView::get_atoms<::tyr::formalism::FluentTag>() const noexcept;
template const boost::dynamic_bitset<>& LiftedStateView::get_atoms<::tyr::formalism::DerivedTag>() const noexcept;

template<::tyr::formalism::FactKind T>
const std::vector<ygg::float_t>& LiftedStateView::get_numeric_variables() const noexcept
{
    if constexpr (std::is_same_v<T, ::tyr::formalism::StaticTag>)
        return m_state_repository->get_task()->get_static_numeric_variables();
    else if constexpr (std::is_same_v<T, ::tyr::formalism::FluentTag>)
        return m_unpacked->get_numeric_variables().values;
    else
        static_assert(ygg::dependent_false<T>::value, "Missing case");
}

template const std::vector<ygg::float_t>& LiftedStateView::get_numeric_variables<::tyr::formalism::StaticTag>() const noexcept;
template const std::vector<ygg::float_t>& LiftedStateView::get_numeric_variables<::tyr::formalism::FluentTag>() const noexcept;

planning::AtomRange<::tyr::formalism::StaticTag> LiftedStateView::get_static_atoms() const noexcept
{
    return planning::AtomRange<::tyr::formalism::StaticTag>(m_state_repository->get_task()->get_static_atoms_bitset());
}

planning::FDRFactRange<planning::LiftedTag, ::tyr::formalism::FluentTag> LiftedStateView::get_fluent_facts() const noexcept
{
    return planning::FDRFactRange<planning::LiftedTag, ::tyr::formalism::FluentTag>(get_atoms<::tyr::formalism::FluentTag>());
}

planning::AtomRange<::tyr::formalism::DerivedTag> LiftedStateView::get_derived_atoms() const noexcept
{
    return planning::AtomRange<::tyr::formalism::DerivedTag>(get_atoms<::tyr::formalism::DerivedTag>());
}

planning::FunctionTermValueRange<::tyr::formalism::StaticTag> LiftedStateView::get_static_fterm_values() const noexcept
{
    return planning::FunctionTermValueRange<::tyr::formalism::StaticTag>(m_state_repository->get_task()->get_static_numeric_variables());
}

planning::FunctionTermValueRange<::tyr::formalism::FluentTag> LiftedStateView::get_fluent_fterm_values() const noexcept
{
    return planning::FunctionTermValueRange<::tyr::formalism::FluentTag>(get_numeric_variables<::tyr::formalism::FluentTag>());
}

const std::shared_ptr<::tyr::formalism::planning::Repository>& LiftedStateView::get_repository() const noexcept
{
    return m_state_repository->get_task()->get_repository();
}

static_assert(planning::IterableStateConcept<LiftedStateView>);
static_assert(planning::IterableViewStateConcept<LiftedStateView>);
static_assert(planning::IndexableStateConcept<LiftedStateView, planning::LiftedTag>);
static_assert(planning::IndexableViewStateConcept<LiftedStateView, planning::LiftedTag>);

}
