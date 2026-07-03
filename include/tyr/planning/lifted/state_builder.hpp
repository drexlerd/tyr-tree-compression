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

#ifndef TYR_PLANNING_LIFTED_STATE_BUILDER_HPP_
#define TYR_PLANNING_LIFTED_STATE_BUILDER_HPP_

#include "tyr/formalism/planning/declarations.hpp"
#include "tyr/formalism/planning/repository.hpp"
#include "tyr/formalism/planning/views.hpp"
#include "tyr/planning/declarations.hpp"
#include "tyr/planning/lifted/state_iterators.hpp"
#include "tyr/planning/lifted/state_storage.hpp"
#include "tyr/planning/state_builder.hpp"
#include "tyr/planning/state_index.hpp"
#include "tyr/planning/state_iterators.hpp"
#include "tyr/planning/state_storage.hpp"

#include <boost/dynamic_bitset.hpp>
#include <vector>
#include <yggdrasil/containers/dynamic_bitset.hpp>
#include <yggdrasil/containers/vector.hpp>
#include <yggdrasil/core/config.hpp>

namespace tyr::planning
{
template<::tyr::formalism::FactKind T>
struct LiftedUnpackedAtomStorageType;

template<>
struct LiftedUnpackedAtomStorageType<::tyr::formalism::FluentTag>
{
    using type = planning::FactUnpackedStorage<LiftedTag>;
};

template<>
struct LiftedUnpackedAtomStorageType<::tyr::formalism::DerivedTag>
{
    using type = planning::AtomUnpackedStorage<LiftedTag>;
};

template<::tyr::formalism::FactKind T>
using LiftedUnpackedAtomStorage = typename LiftedUnpackedAtomStorageType<T>::type;

}

namespace tyr
{

template<>
struct Builder<planning::State<planning::LiftedTag>>
{
public:
    using TaskType = planning::Task<planning::LiftedTag>;

    Builder() = default;

    ygg::Index<planning::State<planning::LiftedTag>> get_index() const;
    void set(ygg::Index<planning::State<planning::LiftedTag>> index);

    void clear();
    void clear_unextended_part();
    void clear_extended_part();
    void assign_unextended_part(const planning::UnpackedState<planning::LiftedTag>& other);

    /**
     * UnpackedStateConcept
     */

    ::tyr::formalism::planning::FDRValue get(ygg::Index<::tyr::formalism::planning::FDRVariable<::tyr::formalism::FluentTag>> index) const;
    void set(ygg::Data<::tyr::formalism::planning::FDRFact<::tyr::formalism::FluentTag>> fact);
    ygg::float_t get(ygg::Index<::tyr::formalism::planning::GroundFunctionTerm<::tyr::formalism::FluentTag>> index) const;
    void set(ygg::Index<::tyr::formalism::planning::GroundFunctionTerm<::tyr::formalism::FluentTag>> index, ygg::float_t value);
    bool test(ygg::Index<::tyr::formalism::planning::GroundAtom<::tyr::formalism::DerivedTag>> index) const;
    void set(ygg::Index<::tyr::formalism::planning::GroundAtom<::tyr::formalism::DerivedTag>> index);

    ::tyr::formalism::planning::FDRValue get(::tyr::formalism::planning::FDRVariableView<::tyr::formalism::FluentTag> view) const;
    void set(::tyr::formalism::planning::FDRFactView<::tyr::formalism::FluentTag> view);
    ygg::float_t get(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::FluentTag> view) const;
    void set(::tyr::formalism::planning::GroundFunctionTermView<::tyr::formalism::FluentTag> view, ygg::float_t value);
    bool test(::tyr::formalism::planning::GroundAtomView<::tyr::formalism::DerivedTag> view) const;
    void set(::tyr::formalism::planning::GroundAtomView<::tyr::formalism::DerivedTag> view);

    planning::FDRFactRange<planning::LiftedTag, ::tyr::formalism::FluentTag> get_fluent_facts() const noexcept;
    planning::AtomRange<::tyr::formalism::DerivedTag> get_derived_atoms() const noexcept;
    planning::FunctionTermValueRange<::tyr::formalism::FluentTag> get_fluent_fterm_values() const noexcept;

    auto get_fluent_facts_view(const ::tyr::formalism::planning::Repository& repository) const noexcept;
    auto get_derived_atoms_view(const ::tyr::formalism::planning::Repository& repository) const noexcept;
    auto get_fluent_fterm_values_view(const ::tyr::formalism::planning::Repository& repository) const noexcept;

    template<::tyr::formalism::FactKind T>
    planning::LiftedUnpackedAtomStorage<T>& get_atoms() noexcept;
    template<::tyr::formalism::FactKind T>
    const planning::LiftedUnpackedAtomStorage<T>& get_atoms() const noexcept;

    planning::NumericUnpackedStorage<planning::LiftedTag>& get_numeric_variables() noexcept;
    const planning::NumericUnpackedStorage<planning::LiftedTag>& get_numeric_variables() const noexcept;

private:
    ygg::Index<planning::State<planning::LiftedTag>> m_index;

    planning::FactUnpackedStorage<planning::LiftedTag> m_fact_storage;
    planning::AtomUnpackedStorage<planning::LiftedTag> m_atom_storage;
    planning::NumericUnpackedStorage<planning::LiftedTag> m_numeric_storage;
};

static_assert(planning::UnpackedStateConcept<planning::UnpackedState<planning::LiftedTag>, planning::LiftedTag>);

inline auto Builder<planning::State<planning::LiftedTag>>::get_fluent_facts_view(const ::tyr::formalism::planning::Repository& repository_) const noexcept
{
    return get_fluent_facts() | std::views::transform([repository = &repository_](auto id) { return ygg::make_view(id, *repository); });
}

inline auto Builder<planning::State<planning::LiftedTag>>::get_derived_atoms_view(const ::tyr::formalism::planning::Repository& repository_) const noexcept
{
    return get_derived_atoms() | std::views::transform([repository = &repository_](auto id) { return ygg::make_view(id, *repository); });
}

inline auto
Builder<planning::State<planning::LiftedTag>>::get_fluent_fterm_values_view(const ::tyr::formalism::planning::Repository& repository_) const noexcept
{
    return get_fluent_fterm_values()
           | std::views::transform([repository = &repository_](auto&& pair) { return std::make_pair(ygg::make_view(pair.first, *repository), pair.second); });
}

}

#endif
