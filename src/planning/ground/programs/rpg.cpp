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

#include "tyr/planning/ground/programs/rpg.hpp"

#include "../../programs/common.hpp"
#include "tyr/formalism/datalog/builder.hpp"
#include "tyr/formalism/datalog/repository.hpp"
#include "tyr/formalism/planning/merge_datalog.hpp"
#include "tyr/formalism/planning/repository.hpp"
#include "tyr/formalism/planning/views.hpp"

namespace f = tyr::formalism;
namespace d = tyr::datalog;
namespace fp = tyr::formalism::planning;
namespace fd = tyr::formalism::datalog;

namespace tyr::planning
{
namespace
{
struct GroundProgramBuildContext
{
    using StaticPredicateMapping = ygg::UnorderedMap<fp::PredicateView<f::StaticTag>, fd::PredicateView<f::StaticTag>>;
    using FluentPredicateMapping = ygg::UnorderedMap<fp::PredicateView<f::FluentTag>, fd::PredicateView<f::FluentTag>>;
    fd::Builder builder;
    fp::MergeDatalogContext merge_context;
    ygg::Data<fd::GroundProgram> program;
    StaticPredicateMapping static_predicates;
    FluentPredicateMapping fluent_predicates;
    ygg::uint_t next_rule_id = 0;

    GroundProgramBuildContext(fd::Repository& repository) : builder(), merge_context(builder, repository), program() { program.clear(); }
};

fd::RuleBindingView create_rule_binding(GroundProgramBuildContext& context, d::Cost cost)
{
    auto predicate_ptr = context.builder.get_builder<f::Predicate<f::FluentTag>>();
    auto& predicate = *predicate_ptr;
    predicate.clear();
    predicate.name = "ground_rule_" + std::to_string(context.next_rule_id++);
    predicate.arity = 0;
    canonicalize(predicate);
    const auto new_predicate = context.merge_context.destination.get_or_create(predicate).first;
    context.program.fluent_predicates.push_back(new_predicate.get_index());

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
    rule.cost = cost;
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

void fill_delete_free_condition(fp::GroundConjunctiveConditionView condition,
                                TranslationContext<GroundTag>& translation_context,
                                GroundProgramBuildContext& context,
                                ygg::Data<fd::GroundConjunctiveCondition>& result)
{
    for (const auto fact : condition.template get_facts<f::PositiveTag>())
        if (const auto literal = fp::merge_p2d(fact, true, translation_context.p2d.fluent_to_fluent_atom, context.fluent_predicates, context.merge_context))
            result.fluent_literals.push_back(literal->get_index());

    for (const auto numeric_constraint : condition.get_numeric_constraints())
        result.numeric_constraints.push_back(fp::merge_p2d(numeric_constraint, context.merge_context));
}

fd::GroundConjunctiveConditionView create_delete_free_condition(fp::GroundConjunctiveConditionView condition,
                                                                TranslationContext<GroundTag>& translation_context,
                                                                GroundProgramBuildContext& context)
{
    auto condition_ptr = context.builder.get_builder<fd::GroundConjunctiveCondition>();
    auto& result = *condition_ptr;
    result.clear();
    fill_delete_free_condition(condition, translation_context, context, result);

    canonicalize(result);
    return context.merge_context.destination.get_or_create(result).first;
}

fd::GroundConjunctiveConditionView
create_delete_free_goal(fp::GroundConjunctiveConditionView goal, TranslationContext<GroundTag>& translation_context, GroundProgramBuildContext& context)
{
    return create_delete_free_condition(goal, translation_context, context);
}

fd::GroundConjunctiveConditionView create_delete_free_effect_condition(fd::GroundAtomView<f::FluentTag> applicability_atom,
                                                                       fp::GroundConjunctiveConditionView action_condition,
                                                                       fp::GroundConjunctiveConditionView effect_condition,
                                                                       TranslationContext<GroundTag>& translation_context,
                                                                       GroundProgramBuildContext& context)
{
    auto condition_ptr = context.builder.get_builder<fd::GroundConjunctiveCondition>();
    auto& result = *condition_ptr;
    result.clear();
    fill_delete_free_condition(effect_condition, translation_context, context, result);
    for (const auto numeric_constraint : action_condition.get_numeric_constraints())
        result.numeric_constraints.push_back(fp::merge_p2d(numeric_constraint, context.merge_context));
    result.fluent_literals.push_back(create_positive_literal(applicability_atom, context).get_index());
    canonicalize(result);
    return context.merge_context.destination.get_or_create(result).first;
}

fd::GroundRuleView
create_ground_atom_rule(fd::GroundConjunctiveConditionView body, fd::GroundAtomView<f::FluentTag> head, GroundProgramBuildContext& context, d::Cost cost)
{
    auto rule_ptr = context.builder.get_builder<fd::GroundRule>();
    auto& rule = *rule_ptr;
    rule.clear();
    rule.binding = create_rule_binding(context, cost).get_index();
    rule.body = body.get_index();
    rule.head = head.get_index();
    canonicalize(rule);
    return context.merge_context.destination.get_or_create(rule).first;
}

fd::GroundRuleView create_ground_numeric_effect_rule(fd::GroundConjunctiveConditionView body,
                                                     fp::GroundNumericEffectOperatorView<f::FluentTag> head,
                                                     GroundProgramBuildContext& context,
                                                     d::Cost cost)
{
    auto rule_ptr = context.builder.get_builder<fd::GroundRule>();
    auto& rule = *rule_ptr;
    rule.clear();
    rule.binding = create_rule_binding(context, cost).get_index();
    rule.body = body.get_index();
    rule.head = fp::merge_p2d(head, context.merge_context);
    canonicalize(rule);
    return context.merge_context.destination.get_or_create(rule).first;
}

fd::GroundRuleView create_applicability_rule(fp::GroundActionView action,
                                             fd::GroundAtomView<f::FluentTag> applicability_atom,
                                             TranslationContext<GroundTag>& translation_context,
                                             GroundProgramBuildContext& context)
{
    return create_ground_atom_rule(create_delete_free_condition(action.get_condition(), translation_context, context), applicability_atom, context, d::Cost(0));
}

fd::ProgramView<GroundTag> finish_program(GroundProgramBuildContext& context)
{
    canonicalize(context.program);
    return context.merge_context.destination.get_or_create(context.program).first;
}

void translate_action_to_delete_free_rules(fp::GroundActionView action,
                                           ygg::Data<fd::GroundProgram>& program,
                                           TranslationContext<GroundTag>& translation_context,
                                           GroundProgramBuildContext& context,
                                           RPGProgram<GroundTag>::RuleToActionMapping& rule_to_action)
{
    const auto applicability_atom = create_applicability_atom(action, context);
    const auto applicability_rule = create_applicability_rule(action, applicability_atom, translation_context, context);
    program.ground_rules.push_back(applicability_rule.get_index());

    for (const auto cond_eff : action.get_effects())
    {
        const auto body =
            create_delete_free_effect_condition(applicability_atom, action.get_condition(), cond_eff.get_condition(), translation_context, context);

        for (const auto fact : cond_eff.get_effect().get_facts<f::PositiveTag>())
        {
            if (const auto literal = fp::merge_p2d(fact, true, translation_context.p2d.fluent_to_fluent_atom, context.fluent_predicates, context.merge_context))
            {
                const auto rule = create_ground_atom_rule(body, literal->get_atom(), context, d::Cost(1));
                program.ground_rules.push_back(rule.get_index());
                rule_to_action.emplace(rule, action);
            }
        }

        for (const auto numeric_effect : cond_eff.get_effect().get_numeric_effects())
        {
            const auto rule = create_ground_numeric_effect_rule(body, numeric_effect, context, d::Cost(1));
            program.ground_rules.push_back(rule.get_index());
            rule_to_action.emplace(rule, action);
        }
    }
}

fd::ProgramView<GroundTag> create_rpg_ground_program(fp::FDRTaskView task,
                                                     TranslationContext<GroundTag>& translation_context,
                                                     RPGProgram<GroundTag>::RuleToActionMapping& mapping,
                                                     fd::Repository& repository)
{
    auto context = GroundProgramBuildContext(repository);
    auto& program = context.program;
    auto& merge_context = context.merge_context;

    for (const auto predicate : task.get_domain().get_predicates<f::StaticTag>())
    {
        const auto new_predicate = fp::merge_p2d(predicate, merge_context).first;
        context.static_predicates.emplace(predicate, new_predicate);
        program.static_predicates.push_back(new_predicate.get_index());
    }
    for (const auto predicate : task.get_domain().get_predicates<f::FluentTag>())
    {
        const auto new_predicate = fp::merge_p2d(predicate, merge_context).first;
        context.fluent_predicates.emplace(predicate, new_predicate);
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
    {
        const auto new_atom = fp::merge_p2d(atom, context.static_predicates, merge_context).first;
        translation_context.p2d.static_to_static_atom.emplace(atom, new_atom);
        translation_context.d2p.static_to_static_atom.emplace(new_atom, atom);
        program.static_atoms.push_back(new_atom.get_index());
    }
    for (const auto atom : task.get_atoms<f::FluentTag>())
    {
        const auto new_atom = fp::merge_p2d(atom, translation_context.p2d.fluent_to_fluent_atom, context.fluent_predicates, merge_context).first;
        translation_context.d2p.fluent_to_fluent_atom.emplace(new_atom, atom);
        program.fluent_atoms.push_back(new_atom.get_index());
    }
    for (const auto fact : task.get_fluent_facts())
    {
        if (const auto atom = fact.get_atom())
        {
            const auto [new_atom, inserted] = fp::merge_p2d(*atom, translation_context.p2d.fluent_to_fluent_atom, context.fluent_predicates, merge_context);
            translation_context.d2p.fluent_to_fluent_atom.emplace(new_atom, *atom);
            if (inserted)
                program.fluent_atoms.push_back(new_atom.get_index());
        }
    }

    for (const auto fterm_value : task.get_fterm_values<f::StaticTag>())
    {
        const auto new_fterm_value = fp::merge_p2d(fterm_value, merge_context).first;
        translation_context.p2d.static_to_static_fterm.emplace(fterm_value.get_fterm(), new_fterm_value.get_fterm());
        translation_context.d2p.static_to_static_fterm.emplace(new_fterm_value.get_fterm(), fterm_value.get_fterm());
        program.static_fterm_values.push_back(new_fterm_value.get_index());
    }
    for (const auto fterm_value : task.get_fterm_values<f::FluentTag>())
    {
        const auto new_fterm_value = fp::merge_p2d(fterm_value, merge_context).first;
        translation_context.p2d.fluent_to_fluent_fterm.emplace(fterm_value.get_fterm(), new_fterm_value.get_fterm());
        translation_context.d2p.fluent_to_fluent_fterm.emplace(new_fterm_value.get_fterm(), fterm_value.get_fterm());
        program.fluent_fterm_values.push_back(new_fterm_value.get_index());
    }

    context.program.goal = create_delete_free_goal(task.get_goal(), translation_context, context).get_index();

    for (const auto action : task.get_ground_actions())
        translate_action_to_delete_free_rules(action, program, translation_context, context, mapping);

    return finish_program(context);
}

d::Program<GroundTag>
create_rpg_datalog_program(fp::FDRTaskView task, TranslationContext<GroundTag>& translation_context, RPGProgram<GroundTag>::RuleToActionMapping& mapping)
{
    auto factory = std::make_shared<fd::RepositoryFactory>();
    auto repository = factory->create_shared();
    auto program = create_rpg_ground_program(task, translation_context, mapping, *repository);
    return d::Program<GroundTag>(program, std::move(repository), std::move(factory));
}

}  // namespace

RPGProgram<GroundTag>::RPGProgram(fp::FDRTaskView task) :
    m_translation_context(),
    m_rule_to_action(),
    m_datalog_program(create_rpg_datalog_program(task, m_translation_context, m_rule_to_action))
{
}

const TranslationContext<GroundTag>& RPGProgram<GroundTag>::get_translation_context() const noexcept { return m_translation_context; }

const RPGProgram<GroundTag>::RuleToActionMapping& RPGProgram<GroundTag>::get_rule_to_action_mapping() const noexcept { return m_rule_to_action; }

fd::ProgramView<GroundTag> RPGProgram<GroundTag>::get_program() const noexcept { return m_datalog_program.get_program(); }

datalog::Program<GroundTag>& RPGProgram<GroundTag>::get_datalog_program() noexcept { return m_datalog_program; }

const datalog::Program<GroundTag>& RPGProgram<GroundTag>::get_datalog_program() const noexcept { return m_datalog_program; }

fd::GroundConjunctiveConditionView RPGProgram<GroundTag>::get_goal() const noexcept { return m_datalog_program.get_program().get_goal().value(); }

}  // namespace tyr::planning
