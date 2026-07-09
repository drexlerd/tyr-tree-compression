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

#ifndef TYR_DATALOG_FACT_SETS_ITERATORS_HPP_
#define TYR_DATALOG_FACT_SETS_ITERATORS_HPP_

#include "tyr/formalism/datalog/repository.hpp"

#include <boost/dynamic_bitset.hpp>
#include <cassert>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>
#include <yggdrasil/core/closed_interval.hpp>
#include <yggdrasil/core/types.hpp>

namespace tyr::datalog
{

template<::tyr::formalism::FactKind T>
class PredicateBindingIndexIterator
{
public:
    using value_type = ygg::Index<::tyr::formalism::RelationBinding<::tyr::formalism::Predicate<T>>>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using pointer = void;

    PredicateBindingIndexIterator() noexcept : m_relation(), m_data(nullptr), m_i(0) {}
    PredicateBindingIndexIterator(ygg::Index<::tyr::formalism::Predicate<T>> relation, const boost::dynamic_bitset<>& data, bool begin) noexcept :
        m_relation(relation),
        m_data(&data),
        m_i(begin ? data.find_first() : boost::dynamic_bitset<>::npos)
    {
    }

    value_type operator*() const noexcept
    {
        assert(m_data);
        return value_type { m_relation, ygg::Index<::tyr::formalism::Row> { static_cast<ygg::uint_t>(m_i) } };
    }

    PredicateBindingIndexIterator& operator++() noexcept
    {
        assert(m_data);
        m_i = m_data->find_next(m_i);
        return *this;
    }

    PredicateBindingIndexIterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const PredicateBindingIndexIterator& lhs, const PredicateBindingIndexIterator& rhs) noexcept
    {
        return lhs.m_data == rhs.m_data && lhs.m_i == rhs.m_i;
    }
    friend bool operator!=(const PredicateBindingIndexIterator& lhs, const PredicateBindingIndexIterator& rhs) noexcept { return !(lhs == rhs); }

private:
    ygg::Index<::tyr::formalism::Predicate<T>> m_relation;
    const boost::dynamic_bitset<>* m_data;
    boost::dynamic_bitset<>::size_type m_i = 0;
};

template<::tyr::formalism::FactKind T>
class PredicateBindingIndexRange : public std::ranges::view_interface<PredicateBindingIndexRange<T>>
{
public:
    PredicateBindingIndexRange() = default;
    PredicateBindingIndexRange(ygg::Index<::tyr::formalism::Predicate<T>> relation, const boost::dynamic_bitset<>& values) :
        m_relation(relation),
        m_data(&values)
    {
    }

    auto begin() const { return PredicateBindingIndexIterator<T>(m_relation, *m_data, true); }
    auto end() const { return PredicateBindingIndexIterator<T>(m_relation, *m_data, false); }

private:
    ygg::Index<::tyr::formalism::Predicate<T>> m_relation;
    const boost::dynamic_bitset<>* m_data = nullptr;
};

template<::tyr::formalism::FactKind T>
class PredicateBindingViewIterator
{
public:
    using value_type = ::tyr::formalism::datalog::PredicateBindingView<T>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using pointer = void;

    PredicateBindingViewIterator() noexcept : m_it(), m_repository(nullptr) {}
    PredicateBindingViewIterator(PredicateBindingIndexIterator<T> it, const ::tyr::formalism::datalog::Repository& repository) noexcept :
        m_it(it),
        m_repository(&repository)
    {
    }

    value_type operator*() const noexcept
    {
        assert(m_repository);
        return ygg::make_view(*m_it, *m_repository);
    }

    PredicateBindingViewIterator& operator++() noexcept
    {
        ++m_it;
        return *this;
    }

    PredicateBindingViewIterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const PredicateBindingViewIterator& lhs, const PredicateBindingViewIterator& rhs) noexcept
    {
        return lhs.m_it == rhs.m_it && lhs.m_repository == rhs.m_repository;
    }
    friend bool operator!=(const PredicateBindingViewIterator& lhs, const PredicateBindingViewIterator& rhs) noexcept { return !(lhs == rhs); }

private:
    PredicateBindingIndexIterator<T> m_it;
    const ::tyr::formalism::datalog::Repository* m_repository;
};

template<::tyr::formalism::FactKind T>
class PredicateBindingViewRange : public std::ranges::view_interface<PredicateBindingViewRange<T>>
{
public:
    PredicateBindingViewRange() = default;
    PredicateBindingViewRange(ygg::Index<::tyr::formalism::Predicate<T>> relation,
                              const boost::dynamic_bitset<>& values,
                              const ::tyr::formalism::datalog::Repository& repository) :
        m_indices(relation, values),
        m_repository(&repository)
    {
    }

    auto begin() const { return PredicateBindingViewIterator<T>(m_indices.begin(), *m_repository); }
    auto end() const { return PredicateBindingViewIterator<T>(m_indices.end(), *m_repository); }

private:
    PredicateBindingIndexRange<T> m_indices;
    const ::tyr::formalism::datalog::Repository* m_repository = nullptr;
};

template<::tyr::formalism::FactKind T>
class FunctionBindingIndexValueIterator
{
public:
    using value_type = std::pair<ygg::Index<::tyr::formalism::RelationBinding<::tyr::formalism::Function<T>>>, ygg::ClosedInterval<ygg::float_t>>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using pointer = void;

    FunctionBindingIndexValueIterator() noexcept : m_relation(), m_bindings(nullptr), m_values(nullptr), m_i(0) {}
    FunctionBindingIndexValueIterator(ygg::Index<::tyr::formalism::Function<T>> relation,
                                      const std::vector<ygg::Index<::tyr::formalism::Row>>& bindings,
                                      const std::vector<ygg::ClosedInterval<ygg::float_t>>& values,
                                      bool begin) noexcept :
        m_relation(relation),
        m_bindings(&bindings),
        m_values(&values),
        m_i(begin ? 0 : bindings.size())
    {
    }

    value_type operator*() const noexcept
    {
        assert(m_bindings);
        assert(m_values);
        return { ygg::Index<::tyr::formalism::RelationBinding<::tyr::formalism::Function<T>>> { m_relation, (*m_bindings)[m_i] }, (*m_values)[m_i] };
    }

    FunctionBindingIndexValueIterator& operator++() noexcept
    {
        ++m_i;
        return *this;
    }

    FunctionBindingIndexValueIterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const FunctionBindingIndexValueIterator& lhs, const FunctionBindingIndexValueIterator& rhs) noexcept
    {
        return lhs.m_bindings == rhs.m_bindings && lhs.m_i == rhs.m_i;
    }
    friend bool operator!=(const FunctionBindingIndexValueIterator& lhs, const FunctionBindingIndexValueIterator& rhs) noexcept { return !(lhs == rhs); }

private:
    ygg::Index<::tyr::formalism::Function<T>> m_relation;
    const std::vector<ygg::Index<::tyr::formalism::Row>>* m_bindings;
    const std::vector<ygg::ClosedInterval<ygg::float_t>>* m_values;
    size_t m_i = 0;
};

template<::tyr::formalism::FactKind T>
class FunctionBindingIndexValueRange : public std::ranges::view_interface<FunctionBindingIndexValueRange<T>>
{
public:
    FunctionBindingIndexValueRange() = default;
    FunctionBindingIndexValueRange(ygg::Index<::tyr::formalism::Function<T>> relation,
                                   const std::vector<ygg::Index<::tyr::formalism::Row>>& bindings,
                                   const std::vector<ygg::ClosedInterval<ygg::float_t>>& values) :
        m_relation(relation),
        m_bindings(&bindings),
        m_values(&values)
    {
    }

    auto begin() const { return FunctionBindingIndexValueIterator<T>(m_relation, *m_bindings, *m_values, true); }
    auto end() const { return FunctionBindingIndexValueIterator<T>(m_relation, *m_bindings, *m_values, false); }

private:
    ygg::Index<::tyr::formalism::Function<T>> m_relation;
    const std::vector<ygg::Index<::tyr::formalism::Row>>* m_bindings = nullptr;
    const std::vector<ygg::ClosedInterval<ygg::float_t>>* m_values = nullptr;
};

template<::tyr::formalism::FactKind T>
class FunctionBindingViewValueIterator
{
public:
    using value_type = std::pair<::tyr::formalism::datalog::FunctionBindingView<T>, ygg::ClosedInterval<ygg::float_t>>;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using pointer = void;

    FunctionBindingViewValueIterator() noexcept : m_it(), m_repository(nullptr) {}
    FunctionBindingViewValueIterator(FunctionBindingIndexValueIterator<T> it, const ::tyr::formalism::datalog::Repository& repository) noexcept :
        m_it(it),
        m_repository(&repository)
    {
    }

    value_type operator*() const noexcept
    {
        assert(m_repository);
        const auto [binding, interval] = *m_it;
        return { ygg::make_view(binding, *m_repository), interval };
    }

    FunctionBindingViewValueIterator& operator++() noexcept
    {
        ++m_it;
        return *this;
    }

    FunctionBindingViewValueIterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const FunctionBindingViewValueIterator& lhs, const FunctionBindingViewValueIterator& rhs) noexcept
    {
        return lhs.m_it == rhs.m_it && lhs.m_repository == rhs.m_repository;
    }
    friend bool operator!=(const FunctionBindingViewValueIterator& lhs, const FunctionBindingViewValueIterator& rhs) noexcept { return !(lhs == rhs); }

private:
    FunctionBindingIndexValueIterator<T> m_it;
    const ::tyr::formalism::datalog::Repository* m_repository;
};

template<::tyr::formalism::FactKind T>
class FunctionBindingViewValueRange : public std::ranges::view_interface<FunctionBindingViewValueRange<T>>
{
public:
    FunctionBindingViewValueRange() = default;
    FunctionBindingViewValueRange(ygg::Index<::tyr::formalism::Function<T>> relation,
                                  const std::vector<ygg::Index<::tyr::formalism::Row>>& bindings,
                                  const std::vector<ygg::ClosedInterval<ygg::float_t>>& values,
                                  const ::tyr::formalism::datalog::Repository& repository) :
        m_indices(relation, bindings, values),
        m_repository(&repository)
    {
    }

    auto begin() const { return FunctionBindingViewValueIterator<T>(m_indices.begin(), *m_repository); }
    auto end() const { return FunctionBindingViewValueIterator<T>(m_indices.end(), *m_repository); }

private:
    FunctionBindingIndexValueRange<T> m_indices;
    const ::tyr::formalism::datalog::Repository* m_repository = nullptr;
};

}

#endif
