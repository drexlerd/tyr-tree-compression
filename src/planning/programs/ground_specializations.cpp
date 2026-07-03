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

#include "common.hpp"
#include "tyr/formalism/datalog/builder.hpp"
#include "tyr/formalism/datalog/repository.hpp"
#include "tyr/formalism/planning/merge_datalog.hpp"
#include "tyr/formalism/planning/repository.hpp"
#include "tyr/formalism/planning/views.hpp"
#include "tyr/planning/ground/programs/action.hpp"
#include "tyr/planning/ground/programs/axiom.hpp"
#include "tyr/planning/ground/programs/rpg.hpp"

namespace f = tyr::formalism;
namespace fp = tyr::formalism::planning;
namespace fd = tyr::formalism::datalog;

namespace tyr::planning
{
namespace
{
struct GroundProgramBuildContext
{
    fd::Builder builder;
    fp::MergeDatalogContext merge_context;
    ygg::Data<fd::GroundProgram> program;
    ygg::uint_t next_rule_id = 0;

    GroundProgramBuildContext(fd::Repository& repository) : builder(), merge_context(builder, repository), program() { program.clear(); }
};

void fill_common_program(fp::FDRTaskView task, TranslationContext& translation_context, GroundProgramBuildContext& context)
{
    auto& program = context.program;
    auto& merge_context = context.merge_context;

    for (const auto predicate : task.get_domain().get_predicates<f::StaticTag>())
    {
        const auto new_predicate = fp::merge_p2d(predicate, merge_context).first;
        translation_context.d2p.static_to_static_predicate.emplace(new_predicate, predicate);
        translation_context.p2d.static_to_static_predicate.emplace(predicate, new_predicate);
        program.static_predicates.push_back(new_predicate.get_index());
    }
    for (const auto predicate : task.get_domain().get_predicates<f::FluentTag>())
    {
        const auto new_predicate = fp::merge_p2d(predicate, merge_context).first;
        translation_context.d2p.fluent_to_fluent_predicate.emplace(new_predicate, predicate);
        translation_context.p2d.fluent_to_fluent_predicate.emplace(predicate, new_predicate);
        program.fluent_predicates.push_back(new_predicate.get_index());
    }
    for (const auto predicate : task.get_domain().get_predicates<f::DerivedTag>())
    {
        const auto new_predicate = fp::merge_p2d<f::DerivedTag, f::FluentTag>(predicate, merge_context).first;
        translation_context.d2p.fluent_to_derived_predicate.emplace(new_predicate, predicate);
        translation_context.p2d.derived_to_fluent_predicate.emplace(predicate, new_predicate);
        program.fluent_predicates.push_back(new_predicate.get_index());
    }
    for (const auto predicate : task.get_derived_predicates())
    {
        const auto new_predicate = fp::merge_p2d<f::DerivedTag, f::FluentTag>(predicate, merge_context).first;
        translation_context.d2p.fluent_to_derived_predicate.emplace(new_predicate, predicate);
        translation_context.p2d.derived_to_fluent_predicate.emplace(predicate, new_predicate);
        program.fluent_predicates.push_back(new_predicate.get_index());
    }

    for (const auto function : task.get_domain().get_functions<f::StaticTag>())
        program.static_functions.push_back(fp::merge_p2d(function, merge_context).first.get_index());
    for (const auto function : task.get_domain().get_functions<f::FluentTag>())
        program.fluent_functions.push_back(fp::merge_p2d(function, merge_context).first.get_index());

    for (const auto object : task.get_domain().get_constants())
        program.objects.push_back(fp::merge_p2d(object, merge_context).first.get_index());
    for (const auto object : task.get_objects())
        program.objects.push_back(fp::merge_p2d(object, merge_context).first.get_index());

    for (const auto atom : task.get_atoms<f::StaticTag>())
        program.static_atoms.push_back(fp::merge_p2d(atom, translation_context.p2d.static_to_static_predicate, merge_context).first.get_index());
    for (const auto atom : task.get_atoms<f::FluentTag>())
        program.fluent_atoms.push_back(fp::merge_p2d(atom, translation_context.p2d.fluent_to_fluent_predicate, merge_context).first.get_index());

    for (const auto fterm_value : task.get_fterm_values<f::StaticTag>())
        program.static_fterm_values.push_back(fp::merge_p2d(fterm_value, merge_context).first.get_index());
    for (const auto fterm_value : task.get_fterm_values<f::FluentTag>())
        program.fluent_fterm_values.push_back(fp::merge_p2d(fterm_value, merge_context).first.get_index());
}

fd::RuleBindingView create_rule_binding(GroundProgramBuildContext& context)
{
    auto predicate_ptr = context.builder.get_builder<f::Predicate<f::FluentTag>>();
    auto& predicate = *predicate_ptr;
    predicate.clear();
    predicate.name = "ground_rule_" + std::to_string(context.next_rule_id++);
    predicate.arity = 0;
    canonicalize(predicate);
    const auto new_predicate = context.merge_context.destination.get_or_create(predicate).first;

    auto atom_ptr = context.builder.get_builder<fd::Atom<f::FluentTag>>();
    auto& atom = *atom_ptr;
    atom.clear();
    atom.predicate = new_predicate.get_index();
    canonicalize(atom);
    const auto new_atom = context.merge_context.destination.get_or_create(atom).first;

    auto condition_ptr = context.builder.get_builder<fd::ConjunctiveCondition>();
    auto& condition = *condition_ptr;
    condition.clear();
    canonicalize(condition);
    const auto new_condition = context.merge_context.destination.get_or_create(condition).first;

    auto rule_ptr = context.builder.get_builder<fd::Rule>();
    auto& rule = *rule_ptr;
    rule.clear();
    rule.body = new_condition.get_index();
    rule.head = new_atom.get_index();
    canonicalize(rule);
    const auto new_rule = context.merge_context.destination.get_or_create(rule).first;

    auto binding_ptr = context.builder.get_builder<f::RelationBinding<fd::Rule>>();
    auto& binding = *binding_ptr;
    binding.clear();
    binding.relation = new_rule.get_index();
    canonicalize(binding);
    return context.merge_context.destination.get_or_create(binding).first;
}

fd::GroundAtomView<f::FluentTag> create_applicability_atom(fp::GroundActionView action, GroundProgramBuildContext& context)
{
    auto predicate_ptr = context.builder.get_builder<f::Predicate<f::FluentTag>>();
    auto& predicate = *predicate_ptr;
    predicate.clear();
    predicate.name = create_applicability_name(action.get_action());
    predicate.arity = action.get_objects().size();
    canonicalize(predicate);
    const auto new_predicate = context.merge_context.destination.get_or_create(predicate).first;

    context.program.fluent_predicates.push_back(new_predicate.get_index());

    auto binding_ptr = context.builder.get_builder<f::RelationBinding<f::Predicate<f::FluentTag>>>();
    auto& binding = *binding_ptr;
    binding.clear();
    binding.relation = new_predicate.get_index();
    for (const auto object : action.get_objects())
        binding.objects.push_back(object.get_index());
    canonicalize(binding);
    const auto new_binding = context.merge_context.destination.get_or_create(binding).first;

    auto atom_ptr = context.builder.get_builder<fd::GroundAtom<f::FluentTag>>();
    auto& atom = *atom_ptr;
    atom.clear();
    atom.binding = new_binding.get_index();
    canonicalize(atom);
    return context.merge_context.destination.get_or_create(atom).first;
}

fd::GroundLiteralView<f::FluentTag> create_positive_literal(fd::GroundAtomView<f::FluentTag> atom, GroundProgramBuildContext& context)
{
    auto literal_ptr = context.builder.get_builder<fd::GroundLiteral<f::FluentTag>>();
    auto& literal = *literal_ptr;
    literal.clear();
    literal.atom = atom.get_index();
    literal.polarity = true;
    canonicalize(literal);
    return context.merge_context.destination.get_or_create(literal).first;
}

fd::GroundConjunctiveConditionView
merge_condition(fp::GroundConjunctiveConditionView condition, const TranslationContext& translation_context, GroundProgramBuildContext& context)
{
    return fp::merge_p2d(condition,
                         translation_context.p2d.fluent_to_fluent_predicate,
                         translation_context.p2d.derived_to_fluent_predicate,
                         context.merge_context)
        .first;
}

fd::GroundConjunctiveConditionView
merge_action_condition(fp::GroundActionView action, const TranslationContext& translation_context, GroundProgramBuildContext& context)
{
    return merge_condition(action.get_condition(), translation_context, context);
}

fd::GroundConjunctiveConditionView merge_applicability_and_condition(fd::GroundAtomView<f::FluentTag> applicability_atom,
                                                                     fp::GroundConjunctiveConditionView condition,
                                                                     const TranslationContext& translation_context,
                                                                     GroundProgramBuildContext& context)
{
    auto merged_condition = merge_condition(condition, translation_context, context);
    auto condition_ptr = context.builder.get_builder<fd::GroundConjunctiveCondition>();
    auto& result = *condition_ptr;
    result.clear();
    result.static_literals = merged_condition.get_literals<f::StaticTag>().get_data();
    result.fluent_literals = merged_condition.get_literals<f::FluentTag>().get_data();
    result.numeric_constraints = merged_condition.get_numeric_constraints().get_data();
    result.fluent_literals.push_back(create_positive_literal(applicability_atom, context).get_index());
    canonicalize(result);
    return context.merge_context.destination.get_or_create(result).first;
}

fd::GroundRuleView create_ground_atom_rule(fd::GroundConjunctiveConditionView body, fd::GroundAtomView<f::FluentTag> head, GroundProgramBuildContext& context)
{
    auto rule_ptr = context.builder.get_builder<fd::GroundRule>();
    auto& rule = *rule_ptr;
    rule.clear();
    rule.binding = create_rule_binding(context).get_index();
    rule.body = body.get_index();
    rule.head = head.get_index();
    canonicalize(rule);
    const auto new_rule = context.merge_context.destination.get_or_create(rule).first;
    context.program.ground_rules.push_back(new_rule.get_index());
    return new_rule;
}

fd::GroundProgramView finish_program(GroundProgramBuildContext& context)
{
    canonicalize(context.program);
    return context.merge_context.destination.get_or_create(context.program).first;
}

fd::GroundProgramView create_action_ground_program(fp::FDRTaskView task,
                                                   TranslationContext& translation_context,
                                                   ApplicableActionProgram<GroundTag>::AppPredicateToActionMapping& mapping,
                                                   fd::Repository& repository)
{
    auto context = GroundProgramBuildContext(repository);
    fill_common_program(task, translation_context, context);

    for (const auto action : task.get_ground_actions())
    {
        const auto app_atom = create_applicability_atom(action, context);
        mapping.emplace(app_atom, action);
        create_ground_atom_rule(merge_action_condition(action, translation_context, context), app_atom, context);
    }

    return finish_program(context);
}

fd::GroundProgramView create_axiom_ground_program(fp::FDRTaskView task, TranslationContext& translation_context, fd::Repository& repository)
{
    auto context = GroundProgramBuildContext(repository);
    fill_common_program(task, translation_context, context);

    for (const auto axiom : task.get_ground_axioms())
    {
        const auto body = merge_condition(axiom.get_body(), translation_context, context);
        const auto head =
            fp::merge_p2d<f::DerivedTag, f::FluentTag>(axiom.get_head(), translation_context.p2d.derived_to_fluent_predicate, context.merge_context).first;
        create_ground_atom_rule(body, head, context);
    }

    return finish_program(context);
}

fd::GroundProgramView create_rpg_ground_program(fp::FDRTaskView task,
                                                TranslationContext& translation_context,
                                                RPGProgram<GroundTag>::GroundRuleToActionMapping& mapping,
                                                fd::Repository& repository)
{
    auto context = GroundProgramBuildContext(repository);
    fill_common_program(task, translation_context, context);
    context.program.goal = merge_condition(task.get_goal(), translation_context, context).get_index();

    for (const auto action : task.get_ground_actions())
    {
        for (const auto cond_eff : action.get_effects())
        {
            const auto body =
                merge_applicability_and_condition(create_applicability_atom(action, context), cond_eff.get_condition(), translation_context, context);

            for (const auto fact : cond_eff.get_effect().get_facts<f::PositiveTag>())
            {
                if (const auto literal = fp::merge_p2d(fact, true, translation_context.p2d.fluent_to_fluent_predicate, context.merge_context))
                {
                    const auto rule = create_ground_atom_rule(body, literal->get_atom(), context);
                    mapping.emplace(rule, action);
                }
            }
        }
    }

    return finish_program(context);
}

}  // namespace

ApplicableActionProgram<GroundTag>::ApplicableActionProgram(fp::FDRTaskView task) :
    m_translation_context(),
    m_ground_atom_to_actions(),
    m_repository_factory(std::make_shared<fd::RepositoryFactory>()),
    m_repository(m_repository_factory->create_shared()),
    m_ground_program(create_action_ground_program(task, m_translation_context, m_ground_atom_to_actions, *m_repository))
{
}

const TranslationContext& ApplicableActionProgram<GroundTag>::get_translation_context() const noexcept { return m_translation_context; }

const ApplicableActionProgram<GroundTag>::AppPredicateToActionMapping& ApplicableActionProgram<GroundTag>::get_ground_atom_to_action_mapping() const noexcept
{
    return m_ground_atom_to_actions;
}

fd::GroundProgramView ApplicableActionProgram<GroundTag>::get_ground_program() const noexcept { return m_ground_program; }

const fd::Repository& ApplicableActionProgram<GroundTag>::get_program_repository() const noexcept { return *m_repository; }

AxiomEvaluatorProgram<GroundTag>::AxiomEvaluatorProgram(fp::FDRTaskView task) :
    m_translation_context(),
    m_repository_factory(std::make_shared<fd::RepositoryFactory>()),
    m_repository(m_repository_factory->create_shared()),
    m_ground_program(create_axiom_ground_program(task, m_translation_context, *m_repository))
{
}

const TranslationContext& AxiomEvaluatorProgram<GroundTag>::get_translation_context() const noexcept { return m_translation_context; }

fd::GroundProgramView AxiomEvaluatorProgram<GroundTag>::get_ground_program() const noexcept { return m_ground_program; }

const fd::Repository& AxiomEvaluatorProgram<GroundTag>::get_program_repository() const noexcept { return *m_repository; }

RPGProgram<GroundTag>::RPGProgram(fp::FDRTaskView task) :
    m_translation_context(),
    m_ground_rule_to_action(),
    m_repository_factory(std::make_shared<fd::RepositoryFactory>()),
    m_repository(m_repository_factory->create_shared()),
    m_ground_program(create_rpg_ground_program(task, m_translation_context, m_ground_rule_to_action, *m_repository))
{
}

const TranslationContext& RPGProgram<GroundTag>::get_translation_context() const noexcept { return m_translation_context; }

const RPGProgram<GroundTag>::GroundRuleToActionMapping& RPGProgram<GroundTag>::get_ground_rule_to_action_mapping() const noexcept
{
    return m_ground_rule_to_action;
}

fd::GroundProgramView RPGProgram<GroundTag>::get_ground_program() const noexcept { return m_ground_program; }

const fd::Repository& RPGProgram<GroundTag>::get_program_repository() const noexcept { return *m_repository; }

fd::GroundConjunctiveConditionView RPGProgram<GroundTag>::get_goal() const noexcept { return m_ground_program.get_goal().value(); }

}  // namespace tyr::planning
