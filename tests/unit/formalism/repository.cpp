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
namespace fd = tyr::formalism::datalog;

namespace tyr::tests
{

TEST(TyrTests, TyrFormalismRepository)
{
    auto factory = fp::RepositoryFactory();
    auto repository = factory.create();
    auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
    auto object_builder = ygg::Data<f::Object>();
    auto atom_builder = ygg::Data<fp::Atom<f::FluentTag>>();

    // Create a unique predicate
    predicate_builder.name = "predicate_0";
    predicate_builder.arity = 2;

    canonicalize(predicate_builder);
    const auto [predicate_0, predicate_success_0] = repository.get_or_create(predicate_builder);

    EXPECT_TRUE(predicate_success_0);
    EXPECT_EQ(predicate_0.get_index().get_value(), 0);
    EXPECT_EQ(predicate_0.get_name(), predicate_builder.name);
    EXPECT_EQ(predicate_0.get_arity(), predicate_builder.arity);

    // Create a unique predicate
    predicate_builder.name = "predicate_1";
    predicate_builder.arity = 3;

    canonicalize(predicate_builder);
    const auto [predicate_1, predicate_success_1] = repository.get_or_create(predicate_builder);

    EXPECT_TRUE(predicate_success_1);
    EXPECT_EQ(predicate_1.get_index().get_value(), 1);
    EXPECT_EQ(predicate_1.get_name(), predicate_builder.name);
    EXPECT_EQ(predicate_1.get_arity(), predicate_builder.arity);

    // Create same predicate again
    predicate_builder.name = "predicate_1";
    predicate_builder.arity = 3;

    canonicalize(predicate_builder);
    const auto [predicate_2, predicate_success_2] = repository.get_or_create(predicate_builder);

    EXPECT_FALSE(predicate_success_2);
    EXPECT_EQ(predicate_2.get_index().get_value(), 1);
    EXPECT_EQ(predicate_2.get_name(), predicate_builder.name);
    EXPECT_EQ(predicate_2.get_arity(), predicate_builder.arity);

    // Create objects
    object_builder.name = "a";
    canonicalize(object_builder);
    const auto [object_0, object_success_0] = repository.get_or_create(object_builder);
    EXPECT_TRUE(object_success_0);
    EXPECT_EQ(object_0.get_name(), object_builder.name);

    object_builder.name = "b";
    canonicalize(object_builder);
    const auto [object_1, object_success_1] = repository.get_or_create(object_builder);
    EXPECT_TRUE(object_success_1);
    EXPECT_EQ(object_1.get_name(), object_builder.name);

    object_builder.name = "c";
    canonicalize(object_builder);
    const auto [object_2, object_success_2] = repository.get_or_create(object_builder);
    EXPECT_TRUE(object_success_2);
    EXPECT_EQ(object_2.get_name(), object_builder.name);

    // Create atom
    atom_builder.terms.clear();
    atom_builder.predicate = predicate_0.get_index();
    atom_builder.terms.push_back(ygg::Data<f::Term>(object_0.get_index()));
    atom_builder.terms.push_back(ygg::Data<f::Term>(object_1.get_index()));
    canonicalize(atom_builder);
    auto [atom_0, atom_success_0] = repository.get_or_create(atom_builder);

    EXPECT_TRUE(atom_success_0);
    // EXPECT_EQ(atom_0.get_terms(), atom_builder.terms);

    // Create same atom again
    canonicalize(atom_builder);
    auto [atom_1, atom_success_1] = repository.get_or_create(atom_builder);
    EXPECT_FALSE(atom_success_1);
}

TEST(TyrTests, TyrFormalismRepositoryViewsUseRepositoryContextForIdentity)
{
    auto factory = fp::RepositoryFactory();
    auto first_repository = factory.create();
    auto second_repository = factory.create();

    auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
    predicate_builder.name = "predicate";
    predicate_builder.arity = 1;
    canonicalize(predicate_builder);

    const auto [first_predicate, first_success] = first_repository.get_or_create(predicate_builder);
    const auto [second_predicate, second_success] = second_repository.get_or_create(predicate_builder);

    EXPECT_TRUE(first_success);
    EXPECT_TRUE(second_success);
    EXPECT_EQ(first_repository.get_index(), 0);
    EXPECT_EQ(second_repository.get_index(), 1);
    EXPECT_EQ(first_predicate.get_index(), second_predicate.get_index());
    EXPECT_FALSE(ygg::EqualTo<fp::PredicateView<f::FluentTag>> {}(first_predicate, second_predicate));
    EXPECT_NE(ygg::Hash<fp::PredicateView<f::FluentTag>> {}(first_predicate), ygg::Hash<fp::PredicateView<f::FluentTag>> {}(second_predicate));
}

TEST(TyrTests, TyrFormalismRepositoryViewsFromIndependentFactoriesUseDeterministicFactoryLocalIdentity)
{
    auto first_factory = fp::RepositoryFactory();
    auto second_factory = fp::RepositoryFactory();
    auto first_repository = first_factory.create();
    auto second_repository = second_factory.create();

    auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
    predicate_builder.name = "predicate";
    predicate_builder.arity = 1;
    canonicalize(predicate_builder);

    const auto [first_predicate, first_success] = first_repository.get_or_create(predicate_builder);
    const auto [second_predicate, second_success] = second_repository.get_or_create(predicate_builder);

    EXPECT_TRUE(first_success);
    EXPECT_TRUE(second_success);
    EXPECT_EQ(first_repository.get_index(), 0);
    EXPECT_EQ(second_repository.get_index(), 0);
    EXPECT_EQ(first_predicate.get_index(), second_predicate.get_index());
    EXPECT_TRUE(ygg::EqualTo<fp::PredicateView<f::FluentTag>> {}(first_predicate, second_predicate));
    EXPECT_EQ(ygg::Hash<fp::PredicateView<f::FluentTag>> {}(first_predicate), ygg::Hash<fp::PredicateView<f::FluentTag>> {}(second_predicate));
}

TEST(TyrTests, TyrDatalogRepositoryViewsFromIndependentFactoriesUseDeterministicFactoryLocalIdentity)
{
    auto first_factory = fd::RepositoryFactory();
    auto second_factory = fd::RepositoryFactory();
    auto first_repository = first_factory.create();
    auto second_repository = second_factory.create();

    auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
    predicate_builder.name = "predicate";
    predicate_builder.arity = 1;
    canonicalize(predicate_builder);

    const auto [first_predicate, first_success] = first_repository.get_or_create(predicate_builder);
    const auto [second_predicate, second_success] = second_repository.get_or_create(predicate_builder);

    EXPECT_TRUE(first_success);
    EXPECT_TRUE(second_success);
    EXPECT_EQ(first_repository.get_index(), 0);
    EXPECT_EQ(second_repository.get_index(), 0);
    EXPECT_EQ(first_predicate.get_index(), second_predicate.get_index());
    EXPECT_TRUE(ygg::EqualTo<fd::PredicateView<f::FluentTag>> {}(first_predicate, second_predicate));
    EXPECT_EQ(ygg::Hash<fd::PredicateView<f::FluentTag>> {}(first_predicate), ygg::Hash<fd::PredicateView<f::FluentTag>> {}(second_predicate));
}

TEST(TyrTests, TyrFormalismPlanningGroundViewsFromIndependentFactoriesUseDeterministicFactoryLocalIdentity)
{
    auto first_factory = fp::RepositoryFactory();
    auto second_factory = fp::RepositoryFactory();
    auto first_repository = first_factory.create();
    auto second_repository = second_factory.create();

    auto make_ground_atom = [](auto& repository)
    {
        auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
        predicate_builder.name = "predicate";
        predicate_builder.arity = 1;
        canonicalize(predicate_builder);
        const auto [predicate, predicate_success] = repository.get_or_create(predicate_builder);
        EXPECT_TRUE(predicate_success);

        auto object_builder = ygg::Data<f::Object>();
        object_builder.name = "object";
        canonicalize(object_builder);
        const auto [object, object_success] = repository.get_or_create(object_builder);
        EXPECT_TRUE(object_success);

        auto binding_builder = ygg::Data<f::RelationBinding<f::Predicate<f::FluentTag>>>();
        binding_builder.relation = predicate.get_index();
        binding_builder.objects.push_back(object.get_index());
        canonicalize(binding_builder);
        const auto [binding, binding_success] = repository.get_or_create(binding_builder);
        EXPECT_TRUE(binding_success);

        auto ground_atom_builder = ygg::Data<fp::GroundAtom<f::FluentTag>>(binding.get_index());
        canonicalize(ground_atom_builder);
        const auto [ground_atom, ground_atom_success] = repository.get_or_create(ground_atom_builder);
        EXPECT_TRUE(ground_atom_success);

        return ground_atom;
    };

    const auto first_ground_atom = make_ground_atom(first_repository);
    const auto second_ground_atom = make_ground_atom(second_repository);

    EXPECT_EQ(first_repository.get_index(), 0);
    EXPECT_EQ(second_repository.get_index(), 0);
    EXPECT_EQ(first_ground_atom.get_index(), second_ground_atom.get_index());
    EXPECT_TRUE(ygg::EqualTo<fp::GroundAtomView<f::FluentTag>> {}(first_ground_atom, second_ground_atom));
    EXPECT_EQ(ygg::Hash<fp::GroundAtomView<f::FluentTag>> {}(first_ground_atom), ygg::Hash<fp::GroundAtomView<f::FluentTag>> {}(second_ground_atom));
}

TEST(TyrTests, TyrDatalogGroundViewsFromIndependentFactoriesUseDeterministicFactoryLocalIdentity)
{
    auto first_factory = fd::RepositoryFactory();
    auto second_factory = fd::RepositoryFactory();
    auto first_repository = first_factory.create();
    auto second_repository = second_factory.create();

    auto make_ground_atom = [](auto& repository)
    {
        auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
        predicate_builder.name = "predicate";
        predicate_builder.arity = 1;
        canonicalize(predicate_builder);
        const auto [predicate, predicate_success] = repository.get_or_create(predicate_builder);
        EXPECT_TRUE(predicate_success);

        auto object_builder = ygg::Data<f::Object>();
        object_builder.name = "object";
        canonicalize(object_builder);
        const auto [object, object_success] = repository.get_or_create(object_builder);
        EXPECT_TRUE(object_success);

        auto binding_builder = ygg::Data<f::RelationBinding<f::Predicate<f::FluentTag>>>();
        binding_builder.relation = predicate.get_index();
        binding_builder.objects.push_back(object.get_index());
        canonicalize(binding_builder);
        const auto [binding, binding_success] = repository.get_or_create(binding_builder);
        EXPECT_TRUE(binding_success);

        auto ground_atom_builder = ygg::Data<fd::GroundAtom<f::FluentTag>>(binding.get_index());
        canonicalize(ground_atom_builder);
        const auto [ground_atom, ground_atom_success] = repository.get_or_create(ground_atom_builder);
        EXPECT_TRUE(ground_atom_success);

        return ground_atom;
    };

    const auto first_ground_atom = make_ground_atom(first_repository);
    const auto second_ground_atom = make_ground_atom(second_repository);

    EXPECT_EQ(first_repository.get_index(), 0);
    EXPECT_EQ(second_repository.get_index(), 0);
    EXPECT_EQ(first_ground_atom.get_index(), second_ground_atom.get_index());
    EXPECT_TRUE(ygg::EqualTo<fd::GroundAtomView<f::FluentTag>> {}(first_ground_atom, second_ground_atom));
    EXPECT_EQ(ygg::Hash<fd::GroundAtomView<f::FluentTag>> {}(first_ground_atom), ygg::Hash<fd::GroundAtomView<f::FluentTag>> {}(second_ground_atom));
}

TEST(TyrTests, TyrDatalogGroundViewsExposeBindingObjects)
{
    auto factory = fd::RepositoryFactory();
    auto repository = factory.create();

    auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
    predicate_builder.name = "predicate";
    predicate_builder.arity = 2;
    canonicalize(predicate_builder);
    const auto [predicate, predicate_success] = repository.get_or_create(predicate_builder);
    ASSERT_TRUE(predicate_success);

    auto function_builder = ygg::Data<f::Function<f::FluentTag>>();
    function_builder.name = "function";
    function_builder.arity = 1;
    canonicalize(function_builder);
    const auto [function, function_success] = repository.get_or_create(function_builder);
    ASSERT_TRUE(function_success);

    auto first_object_builder = ygg::Data<f::Object>();
    first_object_builder.name = "a";
    canonicalize(first_object_builder);
    const auto [first_object, first_object_success] = repository.get_or_create(first_object_builder);
    ASSERT_TRUE(first_object_success);

    auto second_object_builder = ygg::Data<f::Object>();
    second_object_builder.name = "b";
    canonicalize(second_object_builder);
    const auto [second_object, second_object_success] = repository.get_or_create(second_object_builder);
    ASSERT_TRUE(second_object_success);

    auto ground_atom_binding_builder = ygg::Data<f::RelationBinding<f::Predicate<f::FluentTag>>>();
    ground_atom_binding_builder.relation = predicate.get_index();
    ground_atom_binding_builder.objects.push_back(first_object.get_index());
    ground_atom_binding_builder.objects.push_back(second_object.get_index());
    canonicalize(ground_atom_binding_builder);
    const auto [ground_atom_binding, ground_atom_binding_success] = repository.get_or_create(ground_atom_binding_builder);
    ASSERT_TRUE(ground_atom_binding_success);

    auto ground_atom_builder = ygg::Data<fd::GroundAtom<f::FluentTag>>(ground_atom_binding.get_index());
    canonicalize(ground_atom_builder);
    const auto [ground_atom, ground_atom_success] = repository.get_or_create(ground_atom_builder);
    ASSERT_TRUE(ground_atom_success);

    auto ground_function_term_binding_builder = ygg::Data<f::RelationBinding<f::Function<f::FluentTag>>>();
    ground_function_term_binding_builder.relation = function.get_index();
    ground_function_term_binding_builder.objects.push_back(second_object.get_index());
    canonicalize(ground_function_term_binding_builder);
    const auto [ground_function_term_binding, ground_function_term_binding_success] = repository.get_or_create(ground_function_term_binding_builder);
    ASSERT_TRUE(ground_function_term_binding_success);

    auto ground_function_term_builder = ygg::Data<fd::GroundFunctionTerm<f::FluentTag>>(ground_function_term_binding.get_index());
    canonicalize(ground_function_term_builder);
    const auto [ground_function_term, ground_function_term_success] = repository.get_or_create(ground_function_term_builder);
    ASSERT_TRUE(ground_function_term_success);

    auto condition_builder = ygg::Data<fd::GroundConjunctiveCondition>();
    canonicalize(condition_builder);
    const auto [condition, condition_success] = repository.get_or_create(condition_builder);
    ASSERT_TRUE(condition_success);

    auto lifted_atom_builder = ygg::Data<fd::Atom<f::FluentTag>>();
    lifted_atom_builder.predicate = predicate.get_index();
    lifted_atom_builder.terms.push_back(ygg::Data<f::Term>(f::ParameterIndex(0)));
    lifted_atom_builder.terms.push_back(ygg::Data<f::Term>(f::ParameterIndex(1)));
    canonicalize(lifted_atom_builder);
    const auto [lifted_atom, lifted_atom_success] = repository.get_or_create(lifted_atom_builder);
    ASSERT_TRUE(lifted_atom_success);

    auto lifted_condition_builder = ygg::Data<fd::ConjunctiveCondition>();
    canonicalize(lifted_condition_builder);
    const auto [lifted_condition, lifted_condition_success] = repository.get_or_create(lifted_condition_builder);
    ASSERT_TRUE(lifted_condition_success);

    auto rule_builder = ygg::Data<fd::Rule>();
    rule_builder.variables.clear();
    rule_builder.body = lifted_condition.get_index();
    rule_builder.head = lifted_atom.get_index();
    rule_builder.cost = 1;
    canonicalize(rule_builder);
    const auto [rule, rule_success] = repository.get_or_create(rule_builder);
    ASSERT_TRUE(rule_success);

    auto ground_rule_binding_builder = ygg::Data<f::RelationBinding<fd::Rule>>();
    ground_rule_binding_builder.relation = rule.get_index();
    ground_rule_binding_builder.objects.push_back(first_object.get_index());
    ground_rule_binding_builder.objects.push_back(second_object.get_index());
    canonicalize(ground_rule_binding_builder);
    const auto [ground_rule_binding, ground_rule_binding_success] = repository.get_or_create(ground_rule_binding_builder);
    ASSERT_TRUE(ground_rule_binding_success);

    auto ground_rule_builder = ygg::Data<fd::GroundRule>(ygg::Index<fd::GroundRule>(0),
                                                         ground_rule_binding.get_index(),
                                                         condition.get_index(),
                                                         ygg::Data<fd::GroundRule>::Head(ground_atom.get_index()));
    canonicalize(ground_rule_builder);
    const auto [ground_rule, ground_rule_success] = repository.get_or_create(ground_rule_builder);
    ASSERT_TRUE(ground_rule_success);

    const auto ground_atom_objects = ground_atom.get_objects();
    ASSERT_EQ(ground_atom_objects.size(), 2);
    EXPECT_EQ(ground_atom_objects[0].get_index(), first_object.get_index());
    EXPECT_EQ(ground_atom_objects[1].get_index(), second_object.get_index());

    const auto ground_function_term_objects = ground_function_term.get_objects();
    ASSERT_EQ(ground_function_term_objects.size(), 1);
    EXPECT_EQ(ground_function_term_objects[0].get_index(), second_object.get_index());

    const auto ground_rule_objects = ground_rule.get_objects();
    ASSERT_EQ(ground_rule_objects.size(), 2);
    EXPECT_EQ(ground_rule_objects[0].get_index(), first_object.get_index());
    EXPECT_EQ(ground_rule_objects[1].get_index(), second_object.get_index());
}

TEST(TyrTests, TyrFormalismView)
{
    auto factory = fp::RepositoryFactory();
    auto repository = factory.create();
    auto predicate_builder = ygg::Data<f::Predicate<f::FluentTag>>();
    auto object_builder = ygg::Data<f::Object>();
    auto atom_builder = ygg::Data<fp::Atom<f::FluentTag>>();

    // Create a unique predicate
    predicate_builder.name = "predicate_0";
    predicate_builder.arity = 2;
    canonicalize(predicate_builder);
    const auto [predicate_0, predicate_success_0] = repository.get_or_create(predicate_builder);

    // Create objects
    object_builder.name = "a";
    canonicalize(object_builder);
    const auto [object_0, object_success_0] = repository.get_or_create(object_builder);
    object_builder.name = "b";
    canonicalize(object_builder);
    const auto [object_1, object_success_1] = repository.get_or_create(object_builder);

    // Create atom
    atom_builder.terms.clear();
    atom_builder.predicate = predicate_0.get_index();
    atom_builder.terms.push_back(ygg::Data<f::Term>(object_0.get_index()));
    atom_builder.terms.push_back(ygg::Data<f::Term>(object_1.get_index()));
    canonicalize(atom_builder);
    [[maybe_unused]] auto [atom_0, atom_success_0] = repository.get_or_create(atom_builder);
}

}