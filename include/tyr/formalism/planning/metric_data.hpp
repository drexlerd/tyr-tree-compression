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

#ifndef TYR_FORMALISM_PLANNING_METRIC_DATA_HPP_
#define TYR_FORMALISM_PLANNING_METRIC_DATA_HPP_

#include <yggdrasil/core/types.hpp>
#include <yggdrasil/core/types_utils.hpp>
#include "tyr/formalism/planning/declarations.hpp"
#include "tyr/formalism/planning/ground_function_expression_data.hpp"
#include "tyr/formalism/planning/metric_index.hpp"

namespace ygg
{
using namespace ::tyr;


template<>
struct Data<::tyr::formalism::planning::Metric>
{
    using ObjectiveVariant = ::cista::offset::variant<::tyr::formalism::planning::Minimize, ::tyr::formalism::planning::Maximize>;

    ygg::Index<::tyr::formalism::planning::Metric> index;
    ObjectiveVariant objective;
    ygg::Data<::tyr::formalism::planning::GroundFunctionExpression> fexpr;

    Data() = default;
    Data(ObjectiveVariant objective_, ygg::Data<::tyr::formalism::planning::GroundFunctionExpression> fexpr_) : index(), objective(objective_), fexpr(fexpr_) {}
    // Python constructor
    template<typename C>
    Data(ObjectiveVariant objective_, ::ygg::View<ygg::Data<::tyr::formalism::planning::GroundFunctionExpression>, C> fexpr_) : index(), objective(objective_), fexpr()
    {
        ygg::set(fexpr_, fexpr);
    }
    Data(const Data& other) = default;
    Data& operator=(const Data& other) = default;
    Data(Data&& other) = default;
    Data& operator=(Data&& other) = default;

    void clear() noexcept
    {
        ygg::clear(index);
        ygg::clear(objective);
        ygg::clear(fexpr);
    }

    auto cista_members() const noexcept { return std::tie(index, objective, fexpr); }
    auto identifying_members() const noexcept { return std::tie(objective, fexpr); }
};

static_assert(!ygg::uses_trivial_storage_v<::tyr::formalism::planning::Metric>);
}

#endif
