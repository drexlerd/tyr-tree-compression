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

#include "tyr/planning/lifted/programs/rpg.hpp"

#include "../../programs/common.hpp"
#include "tyr/analysis/domains.hpp"
#include "tyr/formalism/datalog/expression_properties.hpp"
#include "tyr/formalism/datalog/formatter.hpp"
#include "tyr/formalism/datalog/repository.hpp"
#include "tyr/formalism/datalog/views.hpp"
#include "tyr/formalism/planning/formatter.hpp"
#include "tyr/formalism/planning/merge_datalog.hpp"
#include "tyr/formalism/planning/repository.hpp"
#include "tyr/formalism/planning/views.hpp"

#include <optional>
#include <yggdrasil/containers/unordered_set.hpp>

namespace f = tyr::formalism;
namespace d = tyr::datalog;
namespace fp = tyr::formalism::planning;
namespace fd = tyr::formalism::datalog;

namespace tyr::planning
{
namespace
{
using MetricFunctionSet = ygg::UnorderedSet<fd::FunctionView<f::FluentTag>>;

bool targets_metric_function(fp::NumericEffectOperatorView<f::FluentTag> effect, const MetricFunctionSet& metric_functions, fp::MergeDatalogContext& context)
{
    return ygg::visit(
        [&](auto&& arg)
        {
            const auto fterm = merge_p2d(arg.get_fterm(), context).first;
            return metric_functions.find(fterm.get_function()) != metric_functions.end();
        },
        effect.get_variant());
}

void fill_delete_free_condition(fp::ActionView action,
                                fp::ConditionalEffectView cond_eff,
                                TranslationContext<LiftedTag>& translation_context,
                                ::tyr::formalism::planning::MergeDatalogContext& context,
                                ygg::Data<::tyr::formalism::datalog::ConjunctiveCondition>& conj_cond)
{
    // Action parameter may get deleted.
    for (const auto& variable : action.get_variables())
        conj_cond.variables.push_back(merge_p2d(variable, context).first.get_index());
    for (const auto literal : action.get_condition().get_literals<::tyr::formalism::StaticTag>())
        conj_cond.static_literals.push_back(merge_p2d(literal, translation_context.p2d.static_to_static_predicate, context).first.get_index());
    for (const auto literal : action.get_condition().get_literals<::tyr::formalism::FluentTag>())
        if (literal.get_polarity())
            conj_cond.fluent_literals.push_back(merge_p2d(literal, translation_context.p2d.fluent_to_fluent_predicate, context).first.get_index());
    for (const auto numeric_constraint : action.get_condition().get_numeric_constraints())
        conj_cond.numeric_constraints.push_back(merge_p2d(numeric_constraint, context));

    for (const auto variable : cond_eff.get_variables())
        conj_cond.variables.push_back(merge_p2d(variable, context).first.get_index());
    for (const auto literal : cond_eff.get_condition().template get_literals<::tyr::formalism::StaticTag>())
        conj_cond.static_literals.push_back(merge_p2d(literal, translation_context.p2d.static_to_static_predicate, context).first.get_index());
    for (const auto literal : cond_eff.get_condition().template get_literals<::tyr::formalism::FluentTag>())
        if (literal.get_polarity())
            conj_cond.fluent_literals.push_back(merge_p2d(literal, translation_context.p2d.fluent_to_fluent_predicate, context).first.get_index());
    for (const auto numeric_constraint : cond_eff.get_condition().get_numeric_constraints())
        conj_cond.numeric_constraints.push_back(merge_p2d(numeric_constraint, context));
}

fd::MetricView create_metric(fp::MetricView metric, fp::MergeDatalogContext& context)
{
    auto metric_ptr = context.builder.get_builder<fd::Metric>();
    auto& result = *metric_ptr;
    result.clear();
    result.fexpr = fp::merge_p2d(metric.get_fexpr(), context);
    canonicalize(result);
    return context.destination.get_or_create(result).first;
}

auto create_delete_free_goal(fp::GroundConjunctiveConditionView goal,
                             TranslationContext<LiftedTag>& translation_context,
                             ::tyr::formalism::planning::MergeDatalogContext& context)
{
    auto conj_cond_ptr = context.builder.get_builder<::tyr::formalism::datalog::GroundConjunctiveCondition>();
    auto& conj_cond = *conj_cond_ptr;
    conj_cond.clear();

    for (const auto fact : goal.get_facts<::tyr::formalism::PositiveTag>())
        if (const auto literal = merge_p2d(fact, true, translation_context.p2d.fluent_to_fluent_predicate, context))
            conj_cond.fluent_literals.push_back(literal->get_index());

    for (const auto numeric_constraint : goal.get_numeric_constraints())
        conj_cond.numeric_constraints.push_back(merge_p2d(numeric_constraint, context));

    canonicalize(conj_cond);
    return context.destination.get_or_create(conj_cond);
}

auto create_conditional_cost(fp::ActionView action,
                             fp::ConditionalEffectView cond_eff,
                             const MetricFunctionSet& metric_functions,
                             TranslationContext<LiftedTag>& translation_context,
                             fp::MergeDatalogContext& context)
{
    auto conj_effect_ptr = context.builder.get_builder<fd::ConjunctiveEffect>();
    auto& conj_effect = *conj_effect_ptr;
    conj_effect.clear();

    for (const auto numeric_effect : cond_eff.get_effect().get_numeric_effects())
        if (targets_metric_function(numeric_effect, metric_functions, context))
            conj_effect.numeric_effects.push_back(merge_p2d(numeric_effect, context));

    if (conj_effect.numeric_effects.empty())
        return std::optional<fd::ConditionalEffectView> {};

    canonicalize(conj_effect);
    const auto effect = context.destination.get_or_create(conj_effect).first;

    auto conj_cond_ptr = context.builder.get_builder<fd::ConjunctiveCondition>();
    auto& conj_cond = *conj_cond_ptr;
    conj_cond.clear();
    fill_delete_free_condition(action, cond_eff, translation_context, context, conj_cond);
    canonicalize(conj_cond);
    const auto condition = context.destination.get_or_create(conj_cond).first;

    auto cond_eff_ptr = context.builder.get_builder<fd::ConditionalEffect>();
    auto& result = *cond_eff_ptr;
    result.clear();
    result.variables = condition.get_variables().get_data();
    result.condition = condition.get_index();
    result.effect = effect.get_index();
    canonicalize(result);
    return std::optional<fd::ConditionalEffectView> { context.destination.get_or_create(result).first };
}

ygg::IndexList<fd::ConditionalEffect> create_conditional_costs(fp::ActionView action,
                                                               const MetricFunctionSet& metric_functions,
                                                               TranslationContext<LiftedTag>& translation_context,
                                                               fp::MergeDatalogContext& context)
{
    auto result = ygg::IndexList<fd::ConditionalEffect> {};
    if (metric_functions.empty())
        return result;

    for (const auto cond_eff : action.get_effects())
        if (const auto conditional_cost = create_conditional_cost(action, cond_eff, metric_functions, translation_context, context))
            result.push_back(conditional_cost->get_index());

    return result;
}

auto create_cond_effect_rule(fp::ActionView action,
                             fp::ConditionalEffectView cond_eff,
                             fp::AtomView<::tyr::formalism::FluentTag> effect,
                             const ygg::IndexList<fd::ConditionalEffect>& conditional_costs,
                             TranslationContext<LiftedTag>& translation_context,
                             ::tyr::formalism::planning::MergeDatalogContext& context)
{
    auto rule_ptr = context.builder.get_builder<::tyr::formalism::datalog::Rule>();
    auto& rule = *rule_ptr;
    rule.clear();

    auto conj_cond_ptr = context.builder.get_builder<::tyr::formalism::datalog::ConjunctiveCondition>();
    auto& conj_cond = *conj_cond_ptr;
    conj_cond.clear();

    fill_delete_free_condition(action, cond_eff, translation_context, context, conj_cond);

    canonicalize(conj_cond);
    const auto new_conj_cond = context.destination.get_or_create(conj_cond).first;

    rule.variables = new_conj_cond.get_variables().get_data();
    rule.body = new_conj_cond.get_index();
    rule.head = merge_p2d(effect, translation_context.p2d.fluent_to_fluent_predicate, context).first.get_index();
    rule.conditional_costs = conditional_costs;

    canonicalize(rule);
    return context.destination.get_or_create(rule);
}

auto create_cond_numeric_effect_rule(fp::ActionView action,
                                     fp::ConditionalEffectView cond_eff,
                                     fp::NumericEffectOperatorView<::tyr::formalism::FluentTag> effect,
                                     const ygg::IndexList<fd::ConditionalEffect>& conditional_costs,
                                     TranslationContext<LiftedTag>& translation_context,
                                     ::tyr::formalism::planning::MergeDatalogContext& context)
{
    auto rule_ptr = context.builder.get_builder<::tyr::formalism::datalog::Rule>();
    auto& rule = *rule_ptr;
    rule.clear();

    auto conj_cond_ptr = context.builder.get_builder<::tyr::formalism::datalog::ConjunctiveCondition>();
    auto& conj_cond = *conj_cond_ptr;
    conj_cond.clear();

    fill_delete_free_condition(action, cond_eff, translation_context, context, conj_cond);

    canonicalize(conj_cond);
    const auto new_conj_cond = context.destination.get_or_create(conj_cond).first;

    rule.variables = new_conj_cond.get_variables().get_data();
    rule.body = new_conj_cond.get_index();
    rule.head = merge_p2d(effect, context);
    rule.conditional_costs = conditional_costs;

    canonicalize(rule);
    return context.destination.get_or_create(rule);
}

void translate_action_to_delete_free_rules(fp::ActionView action,
                                           ygg::Data<fd::Program>& program,
                                           const MetricFunctionSet& metric_functions,
                                           TranslationContext<LiftedTag>& translation_context,
                                           fp::MergeDatalogContext& context,
                                           RPGProgram<LiftedTag>::RuleToActionMapping& rule_to_action)
{
    const auto conditional_costs = create_conditional_costs(action, metric_functions, translation_context, context);

    for (const auto cond_eff : action.get_effects())
    {
        for (const auto literal : cond_eff.get_effect().get_literals())
        {
            if (!literal.get_polarity())
                continue;  /// ignore delete effects

            const auto rule = create_cond_effect_rule(action, cond_eff, literal.get_atom(), conditional_costs, translation_context, context).first;

            program.rules.push_back(rule.get_index());
            rule_to_action.emplace(rule, action);
        }

        for (const auto numeric_effect : cond_eff.get_effect().get_numeric_effects())
        {
            const auto rule = create_cond_numeric_effect_rule(action, cond_eff, numeric_effect, conditional_costs, translation_context, context).first;

            program.rules.push_back(rule.get_index());
            rule_to_action.emplace(rule, action);
        }
    }
}

auto create_program(fp::TaskView task,
                    TranslationContext<LiftedTag>& translation_context,
                    RPGProgram<LiftedTag>::RuleToActionMapping& rule_to_action,
                    fd::Repository& destination)
{
    auto builder = fd::Builder();
    auto context = fp::MergeDatalogContext(builder, destination);
    auto program_ptr = builder.get_builder<fd::Program>();
    auto& program = *program_ptr;
    program.clear();

    for (const auto predicate : task.get_domain().get_predicates<f::StaticTag>())
    {
        const auto new_predicate = fp::merge_p2d(predicate, context).first;
        translation_context.d2p.static_to_static_predicate.emplace(new_predicate, predicate);
        translation_context.p2d.static_to_static_predicate.emplace(predicate, new_predicate);
        program.static_predicates.push_back(new_predicate.get_index());
    }
    for (const auto predicate : task.get_domain().get_predicates<f::FluentTag>())
    {
        const auto new_predicate = fp::merge_p2d(predicate, context).first;
        translation_context.d2p.fluent_to_fluent_predicate.emplace(new_predicate, predicate);
        translation_context.p2d.fluent_to_fluent_predicate.emplace(predicate, new_predicate);
        program.fluent_predicates.push_back(new_predicate.get_index());
    }

    for (const auto function : task.get_domain().get_functions<f::StaticTag>())
        program.static_functions.push_back(fp::merge_p2d(function, context).first.get_index());
    for (const auto function : task.get_domain().get_functions<f::FluentTag>())
        program.fluent_functions.push_back(fp::merge_p2d(function, context).first.get_index());

    for (const auto object : task.get_domain().get_constants())
        program.objects.push_back(fp::merge_p2d(object, context).first.get_index());
    for (const auto object : task.get_objects())
        program.objects.push_back(fp::merge_p2d(object, context).first.get_index());

    for (const auto atom : task.get_atoms<f::StaticTag>())
        program.static_atoms.push_back(fp::merge_p2d(atom, translation_context.p2d.static_to_static_predicate, context).first.get_index());
    for (const auto atom : task.get_atoms<f::FluentTag>())
        program.fluent_atoms.push_back(fp::merge_p2d(atom, translation_context.p2d.fluent_to_fluent_predicate, context).first.get_index());

    for (const auto fterm_value : task.get_fterm_values<f::StaticTag>())
        program.static_fterm_values.push_back(fp::merge_p2d(fterm_value, context).first.get_index());
    for (const auto fterm_value : task.get_fterm_values<f::FluentTag>())
        program.fluent_fterm_values.push_back(fp::merge_p2d(fterm_value, context).first.get_index());

    program.goal = create_delete_free_goal(task.get_goal(), translation_context, context).first.get_index();
    auto metric_functions = MetricFunctionSet {};
    if (task.get_metric())
    {
        const auto metric = create_metric(task.get_metric().value(), context);
        program.metric = metric.get_index();

        auto metric_fterms = ygg::UnorderedSet<fd::GroundFunctionTermView<f::FluentTag>> {};
        fd::collect_fterms(metric.get_fexpr(), metric_fterms);
        for (const auto fterm : metric_fterms)
            metric_functions.insert(fterm.get_function());
    }

    for (const auto action : task.get_domain().get_actions())
        translate_action_to_delete_free_rules(action, program, metric_functions, translation_context, context, rule_to_action);

    canonicalize(program);
    return destination.get_or_create(program).first;
}

auto create_datalog_program(fp::TaskView task, TranslationContext<LiftedTag>& translation_context, RPGProgram<LiftedTag>::RuleToActionMapping& rule_to_action)
{
    auto factory = std::make_shared<fd::RepositoryFactory>();
    auto repository = factory->create_shared();
    auto program = create_program(task, translation_context, rule_to_action, *repository);
    auto domains = analysis::compute_variable_domains(program);
    auto strata = analysis::compute_rule_stratification(program);
    auto listeners = analysis::compute_listeners(strata, *repository);

    return datalog::Program<LiftedTag>(program, std::move(repository), std::move(factory), std::move(domains), std::move(strata), std::move(listeners));
}

}

RPGProgram<LiftedTag>::RPGProgram(fp::TaskView task) :
    m_translation_context(),
    m_rule_to_action(),
    m_datalog_program(create_datalog_program(task, m_translation_context, m_rule_to_action))
{
    // std::cout << m_datalog_program.get_program() << std::endl;
}

const TranslationContext<LiftedTag>& RPGProgram<LiftedTag>::get_translation_context() const noexcept { return m_translation_context; }

const RPGProgram<LiftedTag>::RuleToActionMapping& RPGProgram<LiftedTag>::get_rule_to_action_mapping() const noexcept { return m_rule_to_action; }

datalog::Program<LiftedTag>& RPGProgram<LiftedTag>::get_datalog_program() noexcept { return m_datalog_program; }

const datalog::Program<LiftedTag>& RPGProgram<LiftedTag>::get_datalog_program() const noexcept { return m_datalog_program; }

const datalog::ConstProgramWorkspace<LiftedTag>& RPGProgram<LiftedTag>::get_const_program_workspace() const noexcept
{
    return m_datalog_program.get_const_program_workspace();
}

::tyr::formalism::datalog::GroundConjunctiveConditionView RPGProgram<LiftedTag>::get_goal() const noexcept
{
    return m_datalog_program.get_program().get_goal().value();
}

}
