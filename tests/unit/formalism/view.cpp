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

#include "tyr/formalism/formalism.hpp"

#include <gtest/gtest.h>

namespace b = ygg::buffer;
namespace f = tyr::formalism;
namespace fp = tyr::formalism::planning;

namespace tyr::tests
{

TEST(TyrTests, TyrFormalismView)
{
    auto factory = fp::RepositoryFactory();
    auto repository = factory.create();
    auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
    auto function_builder = ygg::Data<f::Function<f::FluentTag>>();
    auto object_builder = ygg::Data<f::Object>();
    auto atom_builder = ygg::Data<fp::Atom<f::FluentTag>>();

    // Create a unique predicate
    predicate_builder.name = "predicate";
    predicate_builder.arity = 2;
    canonicalize(predicate_builder);
    auto [predicate, predicate_success] = repository.get_or_create(predicate_builder);
    ASSERT_TRUE(predicate_success);

    // Create a unique function
    function_builder.name = "function";
    function_builder.arity = 1;
    canonicalize(function_builder);
    auto [function, function_success] = repository.get_or_create(function_builder);
    ASSERT_TRUE(function_success);

    // Create object and variable
    object_builder.name = "a";
    canonicalize(object_builder);
    auto [object, object_success] = repository.get_or_create(object_builder);
    ASSERT_TRUE(object_success);

    // Create atom
    atom_builder.terms.clear();
    atom_builder.predicate = predicate.get_index();
    atom_builder.terms.push_back(ygg::Data<f::Term>(object.get_index()));
    atom_builder.terms.push_back(ygg::Data<f::Term>(f::ParameterIndex(0)));
    canonicalize(atom_builder);
    auto [atom, atom_success] = repository.get_or_create(atom_builder);
    ASSERT_TRUE(atom_success);

    // Recurse through proxy
    auto atom_predicate = atom.get_predicate();
    auto atom_terms = atom.get_terms();

    ASSERT_EQ(atom_terms.size(), 2);
    EXPECT_EQ(atom_predicate.get_name(), "predicate");
    EXPECT_EQ(atom_predicate.get_arity(), 2);
    visit(
        [&](auto&& arg)
        {
            using Alternative = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<Alternative, fp::ObjectView>)
                EXPECT_EQ(arg.get_index(), object.get_index());
            else
                FAIL() << "Expected ObjectView for first term, got a different proxy type";
        },
        atom_terms[0].get_variant());
    visit(
        [&](auto&& arg)
        {
            using Alternative = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<Alternative, f::ParameterIndex>)
                EXPECT_EQ(arg, f::ParameterIndex(0));
            else
                FAIL() << "Expected VariableView for first term, got a different proxy type";
        },
        atom_terms[1].get_variant());

    auto ground_atom_binding_builder = ygg::Data<f::RelationBinding<f::Predicate<f::FluentTag>>>();
    ground_atom_binding_builder.relation = predicate.get_index();
    ground_atom_binding_builder.objects.push_back(object.get_index());
    ground_atom_binding_builder.objects.push_back(object.get_index());
    canonicalize(ground_atom_binding_builder);
    auto [ground_atom_binding, ground_atom_binding_success] = repository.get_or_create(ground_atom_binding_builder);
    ASSERT_TRUE(ground_atom_binding_success);

    auto ground_atom_builder = ygg::Data<fp::GroundAtom<f::FluentTag>>(ground_atom_binding.get_index());
    canonicalize(ground_atom_builder);
    auto [ground_atom, ground_atom_success] = repository.get_or_create(ground_atom_builder);
    ASSERT_TRUE(ground_atom_success);

    auto function_binding_builder = ygg::Data<f::RelationBinding<f::Function<f::FluentTag>>>();
    function_binding_builder.relation = function.get_index();
    function_binding_builder.objects.push_back(object.get_index());
    canonicalize(function_binding_builder);
    auto [function_binding, function_binding_success] = repository.get_or_create(function_binding_builder);
    ASSERT_TRUE(function_binding_success);

    auto ground_function_term_builder = ygg::Data<fp::GroundFunctionTerm<f::FluentTag>>(function_binding.get_index());
    canonicalize(ground_function_term_builder);
    auto [ground_function_term, ground_function_term_success] = repository.get_or_create(ground_function_term_builder);
    ASSERT_TRUE(ground_function_term_success);

    auto ground_atom_objects = ground_atom.get_objects();
    ASSERT_EQ(ground_atom_objects.size(), 2);
    EXPECT_EQ(ground_atom_objects[0].get_index(), object.get_index());
    EXPECT_EQ(ground_atom_objects[1].get_index(), object.get_index());
    EXPECT_EQ(ground_atom.get_predicate().get_index(), predicate.get_index());

    auto ground_function_term_objects = ground_function_term.get_objects();
    ASSERT_EQ(ground_function_term_objects.size(), 1);
    EXPECT_EQ(ground_function_term_objects[0].get_index(), object.get_index());
    EXPECT_EQ(ground_function_term.get_function().get_index(), function.get_index());
}

}
