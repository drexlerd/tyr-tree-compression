

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

#ifndef TYR_FORMALISM_DATALOG_CANONICALIZATION_HPP_
#define TYR_FORMALISM_DATALOG_CANONICALIZATION_HPP_

#include <yggdrasil/semantics/canonicalization.hpp>
#include <yggdrasil/semantics/comparators.hpp>
#include "tyr/formalism/canonicalization.hpp"
#include "tyr/formalism/datalog/datas.hpp"

#include <algorithm>

namespace tyr::formalism::datalog
{

/**
 * Datalog
 */

template<OpKind Op, typename T>
bool is_canonical(const ygg::Data<UnaryOperator<Op, T>>& data)
{
    return true;
}

template<OpKind Op, typename T>
bool is_canonical(const ygg::Data<BinaryOperator<Op, T>>& data)
{
    return true;
}

template<typename T>
bool is_canonical(const ygg::Data<BinaryOperator<Add, T>>& data)
{
    return ygg::LessEqual<T> {}(data.lhs, data.rhs);
}

template<typename T>
bool is_canonical(const ygg::Data<BinaryOperator<Mul, T>>& data)
{
    return ygg::LessEqual<T> {}(data.lhs, data.rhs);
}

template<OpKind Op, typename T>
bool is_canonical(const ygg::Data<MultiOperator<Op, T>>& data)
{
    return true;
}

template<typename T>
bool is_canonical(const ygg::Data<MultiOperator<Add, T>>& data)
{
    return is_canonical(data.args);
}

template<typename T>
bool is_canonical(const ygg::Data<MultiOperator<Mul, T>>& data)
{
    return is_canonical(data.args);
}

template<typename T>
bool is_canonical(const ygg::Data<BooleanOperator<T>>& data)
{
    return true;
}

template<typename T>
bool is_canonical(const ygg::Data<ArithmeticOperator<T>>& data)
{
    return true;
}

template<FactKind T>
bool is_canonical(const ygg::Data<Atom<T>>& data)
{
    return true;
}

template<FactKind T>
bool is_canonical(const ygg::Data<Literal<T>>& data)
{
    return true;
}

template<FactKind T>
bool is_canonical(const ygg::Data<GroundAtom<T>>& data)
{
    return true;
}

template<FactKind T>
bool is_canonical(const ygg::Data<GroundLiteral<T>>& data)
{
    return true;
}

template<FactKind T>
bool is_canonical(const ygg::Data<FunctionTerm<T>>& data)
{
    return true;
}

inline bool is_canonical(const ygg::Data<FunctionExpression>& data) { return true; }

template<FactKind T>
bool is_canonical(const ygg::Data<GroundFunctionTerm<T>>& data)
{
    return true;
}

inline bool is_canonical(const ygg::Data<GroundFunctionExpression>& data) { return true; }

template<FactKind T>
bool is_canonical(const ygg::Data<GroundFunctionTermValue<T>>& data)
{
    return true;
}

template<NumericEffectOpKind Op, FactKind T>
bool is_canonical(const ygg::Data<NumericEffect<Op, T>>& data)
{
    return true;
}

template<NumericEffectOpKind Op, FactKind T>
bool is_canonical(const ygg::Data<GroundNumericEffect<Op, T>>& data)
{
    return true;
}

inline bool is_canonical(const ygg::Data<ConjunctiveCondition>& data)
{
    return is_canonical(data.static_literals) && is_canonical(data.fluent_literals) && is_canonical(data.numeric_constraints);
}

inline bool is_canonical(const ygg::Data<GroundConjunctiveCondition>& data)
{
    return is_canonical(data.static_literals) && is_canonical(data.fluent_literals) && is_canonical(data.numeric_constraints);
}

inline bool is_canonical(const ygg::Data<Rule>& data) { return true; }

inline bool is_canonical(const ygg::Data<GroundRule>& data) { return true; }

inline bool is_canonical(const ygg::Data<Program>& data)
{
    return is_canonical(data.static_predicates) && is_canonical(data.fluent_predicates) && is_canonical(data.static_functions)
           && is_canonical(data.fluent_functions) && is_canonical(data.objects) && is_canonical(data.static_atoms) && is_canonical(data.fluent_atoms)
           && is_canonical(data.static_fterm_values) && is_canonical(data.fluent_fterm_values) && is_canonical(data.goal) && is_canonical(data.rules);
}

/**
 * Datalog
 */

template<OpKind Op, typename T>
void canonicalize(ygg::Data<UnaryOperator<Op, T>>& data)
{
    // Trivially canonical
}

template<OpKind Op, typename T>
void canonicalize(ygg::Data<BinaryOperator<Op, T>>& data)
{
    // Canonicalization for commutative operator in spezializations
}

template<typename T>
void canonicalize(ygg::Data<BinaryOperator<Add, T>>& data)
{
    if (ygg::Greater<T> {}(data.lhs, data.rhs))
        std::swap(data.lhs, data.rhs);
}

template<typename T>
void canonicalize(ygg::Data<BinaryOperator<Mul, T>>& data)
{
    if (ygg::Greater<T> {}(data.lhs, data.rhs))
        std::swap(data.lhs, data.rhs);
}

template<OpKind Op, typename T>
void canonicalize(ygg::Data<MultiOperator<Op, T>>& data)
{
    // Canonicalization for commutative operator in spezializations
}

template<typename T>
void canonicalize(ygg::Data<MultiOperator<Add, T>>& data)
{
    canonicalize(data.args);
}

template<typename T>
void canonicalize(ygg::Data<MultiOperator<Mul, T>>& data)
{
    canonicalize(data.args);
}

template<typename T>
void canonicalize(ygg::Data<BooleanOperator<T>>& data)
{
    // Trivially canonical
}

template<typename T>
void canonicalize(ygg::Data<ArithmeticOperator<T>>& data)
{
    // Trivially canonical
}

template<FactKind T>
void canonicalize(ygg::Data<Atom<T>>& data)
{
    // Trivially canonical
}

template<FactKind T>
void canonicalize(ygg::Data<Literal<T>>& data)
{
    // Trivially canonical
}

template<FactKind T>
void canonicalize(ygg::Data<GroundAtom<T>>& data)
{
    // Trivially canonical
}

template<FactKind T>
void canonicalize(ygg::Data<GroundLiteral<T>>& data)
{
    // Trivially canonical
}

template<FactKind T>
void canonicalize(ygg::Data<FunctionTerm<T>>& data)
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<FunctionExpression>& data)
{
    // Trivially canonical
}

template<FactKind T>
void canonicalize(ygg::Data<GroundFunctionTerm<T>>& data)
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<GroundFunctionExpression>& data)
{
    // Trivially canonical
}

template<FactKind T>
void canonicalize(ygg::Data<GroundFunctionTermValue<T>>& data)
{
    // Trivially canonical
}

template<NumericEffectOpKind Op, FactKind T>
void canonicalize(ygg::Data<NumericEffect<Op, T>>& data)
{
    // Trivially canonical
}

template<NumericEffectOpKind Op, FactKind T>
void canonicalize(ygg::Data<GroundNumericEffect<Op, T>>& data)
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<ConjunctiveCondition>& data)
{
    canonicalize(data.static_literals);
    canonicalize(data.fluent_literals);
    canonicalize(data.numeric_constraints);
}

inline void canonicalize(ygg::Data<GroundConjunctiveCondition>& data)
{
    canonicalize(data.static_literals);
    canonicalize(data.fluent_literals);
    canonicalize(data.numeric_constraints);
}

inline void canonicalize(ygg::Data<Rule>& data)
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<GroundRule>& data)
{
    // Trivially canonical
}

inline void canonicalize(ygg::Data<Program>& data)
{
    canonicalize(data.static_predicates);
    canonicalize(data.fluent_predicates);
    canonicalize(data.static_functions);
    canonicalize(data.fluent_functions);
    canonicalize(data.objects);
    canonicalize(data.static_atoms);
    canonicalize(data.fluent_atoms);
    canonicalize(data.static_fterm_values);
    canonicalize(data.fluent_fterm_values);
    canonicalize(data.goal);
    canonicalize(data.rules);
}

}

#endif
