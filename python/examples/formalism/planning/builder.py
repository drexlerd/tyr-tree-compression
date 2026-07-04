"""
Create planning formalism structures.

This example demonstrates how to create planning tasks over the gripper domain.

Example usage (run from the repository root):

    python3 python/examples/formalism/planning/builder.py

Author: Dominik Drexler (dominik.drexler@liu.se)
"""

from pyyggdrasil.execution import ExecutionContext

from pytyr.formalism.planning import (
    ParameterIndex,
    RepositoryFactory,
    ObjectData,
    VariableData,
    TermData,
    StaticPredicateData,
    FluentPredicateData,
    StaticPredicateBindingData,
    FluentPredicateBindingData,
    StaticAtomData,
    FluentAtomData,
    StaticGroundAtomData,
    FluentGroundAtomData,
    StaticLiteralData,
    FluentLiteralData,
    ConjunctiveConditionData,
    ConjunctiveEffectData,
    ConditionalEffectData,
    ActionData,
    DomainData,
    GroundConjunctiveConditionData,
    LiftedTaskData,
    FDRContext,
    PlanningDomain,
    PlanningTask,
)

from pytyr.planning.lifted import (
    Task,
    GroundTaskInstantiationOptions,
)


def create(repository, builder):
    return repository.create(builder)


def get(repository, builder):
    value, _ = repository.get_or_create(builder)
    return value


def make_term(repository, value):
    return get(repository, TermData(value))


def make_static_atom(repository, predicate, terms):
    return get(repository, StaticAtomData(predicate, terms))


def make_fluent_atom(repository, predicate, terms):
    return get(repository, FluentAtomData(predicate, terms))


def make_static_literal(repository, predicate, terms, polarity=True):
    atom = make_static_atom(repository, predicate, terms)
    return get(repository, StaticLiteralData(atom, polarity))


def make_fluent_literal(repository, predicate, terms, polarity=True):
    atom = make_fluent_atom(repository, predicate, terms)
    return get(repository, FluentLiteralData(atom, polarity))


def make_static_ground_atom(repository, predicate, objects):
    binding = get(repository, StaticPredicateBindingData(predicate, objects))
    return get(repository, StaticGroundAtomData(binding))


def make_fluent_ground_atom(repository, predicate, objects):
    binding = get(repository, FluentPredicateBindingData(predicate, objects))
    return get(repository, FluentGroundAtomData(binding))


def main():
    factory = RepositoryFactory()

    # --------------------------------------------------------------------------
    # 1. Build the domain
    # --------------------------------------------------------------------------

    # Create a root repository
    domain_repository = factory.create_repository()

    # Static predicates
    room = get(domain_repository, StaticPredicateData("room", 1))
    ball = get(domain_repository, StaticPredicateData("ball", 1))
    gripper = get(domain_repository, StaticPredicateData("gripper", 1))

    # Fluent predicates
    at_robby = get(domain_repository, FluentPredicateData("at-robby", 1))
    at = get(domain_repository, FluentPredicateData("at", 2))
    free = get(domain_repository, FluentPredicateData("free", 1))
    carry = get(domain_repository, FluentPredicateData("carry", 2))

    # Constants
    rooma = get(domain_repository, ObjectData("rooma"))
    roomb = get(domain_repository, ObjectData("roomb"))

    # --------------------------------------------------------------------------
    # Lifted variables
    # --------------------------------------------------------------------------

    v_from = get(domain_repository, VariableData("?from"))
    v_to = get(domain_repository, VariableData("?to"))

    v_obj = get(domain_repository, VariableData("?obj"))
    v_room = get(domain_repository, VariableData("?room"))
    v_gripper = get(domain_repository, VariableData("?gripper"))

    # Terms
    t_from = create(domain_repository, TermData(ParameterIndex(0)))
    t_to = create(domain_repository, TermData(ParameterIndex(1)))

    t_obj = create(domain_repository, TermData(ParameterIndex(0)))
    t_room = create(domain_repository, TermData(ParameterIndex(1)))
    t_gripper = create(domain_repository, TermData(ParameterIndex(2)))

    # --------------------------------------------------------------------------
    # move action
    # Preconditions:
    #   (room ?from) (room ?to) (at-robby ?from)
    # Effects:
    #   (at-robby ?to) and not (at-robby ?from)
    # --------------------------------------------------------------------------

    move_condition = get(
        domain_repository,
        ConjunctiveConditionData(
            variables=[v_from, v_to],
            static_literals=[
                make_static_literal(domain_repository, room, [t_from], True),
                make_static_literal(domain_repository, room, [t_to], True),
            ],
            fluent_literals=[
                make_fluent_literal(domain_repository, at_robby, [t_from], True),
            ],
            derived_literals=[],
            numeric_constraints=[],
        ),
    )

    move_effect = get(
        domain_repository,
        ConjunctiveEffectData(
            fluent_literals=[
                make_fluent_literal(domain_repository, at_robby, [t_to], True),
                make_fluent_literal(domain_repository, at_robby, [t_from], False),
            ],
            fluent_numeric_effects=[],
            auxiliary_numeric_effect=None,
        ),
    )

    move_conditional_effect = get(
        domain_repository,
        ConditionalEffectData(
            variables=[],
            condition=get(
                domain_repository,
                ConjunctiveConditionData(
                    variables=[],
                    static_literals=[],
                    fluent_literals=[],
                    derived_literals=[],
                    numeric_constraints=[],
                ),
            ),
            effect=move_effect,
        ),
    )

    move = get(
        domain_repository,
        ActionData(
            name="move",
            original_arity=2,
            variables=[v_from, v_to],
            condition=move_condition,
            effects=[move_conditional_effect],
        ),
    )

    # --------------------------------------------------------------------------
    # pick action
    # Preconditions:
    #   (ball ?obj) (room ?room) (gripper ?gripper)
    #   (at ?obj ?room) (at-robby ?room) (free ?gripper)
    # Effects:
    #   (carry ?obj ?gripper)
    #   not (at ?obj ?room)
    #   not (free ?gripper)
    # --------------------------------------------------------------------------

    pick_condition = get(
        domain_repository,
        ConjunctiveConditionData(
            variables=[v_obj, v_room, v_gripper],
            static_literals=[
                make_static_literal(domain_repository, ball, [t_obj], True),
                make_static_literal(domain_repository, room, [t_room], True),
                make_static_literal(domain_repository, gripper, [t_gripper], True),
            ],
            fluent_literals=[
                make_fluent_literal(domain_repository, at, [t_obj, t_room], True),
                make_fluent_literal(domain_repository, at_robby, [t_room], True),
                make_fluent_literal(domain_repository, free, [t_gripper], True),
            ],
            derived_literals=[],
            numeric_constraints=[],
        ),
    )

    pick_effect = get(
        domain_repository,
        ConjunctiveEffectData(
            fluent_literals=[
                make_fluent_literal(domain_repository, carry, [t_obj, t_gripper], True),
                make_fluent_literal(domain_repository, at, [t_obj, t_room], False),
                make_fluent_literal(domain_repository, free, [t_gripper], False),
            ],
            fluent_numeric_effects=[],
            auxiliary_numeric_effect=None,
        ),
    )

    pick_conditional_effect = get(
        domain_repository,
        ConditionalEffectData(
            variables=[],
            condition=get(
                domain_repository,
                ConjunctiveConditionData(
                    variables=[],
                    static_literals=[],
                    fluent_literals=[],
                    derived_literals=[],
                    numeric_constraints=[],
                ),
            ),
            effect=pick_effect,
        ),
    )

    pick = get(
        domain_repository,
        ActionData(
            name="pick",
            original_arity=3,
            variables=[v_obj, v_room, v_gripper],
            condition=pick_condition,
            effects=[pick_conditional_effect],
        ),
    )

    # --------------------------------------------------------------------------
    # drop action
    # Preconditions:
    #   (ball ?obj) (room ?room) (gripper ?gripper)
    #   (carry ?obj ?gripper) (at-robby ?room)
    # Effects:
    #   (at ?obj ?room)
    #   (free ?gripper)
    #   not (carry ?obj ?gripper)
    # --------------------------------------------------------------------------

    drop_condition = get(
        domain_repository,
        ConjunctiveConditionData(
            variables=[v_obj, v_room, v_gripper],
            static_literals=[
                make_static_literal(domain_repository, ball, [t_obj], True),
                make_static_literal(domain_repository, room, [t_room], True),
                make_static_literal(domain_repository, gripper, [t_gripper], True),
            ],
            fluent_literals=[
                make_fluent_literal(domain_repository, carry, [t_obj, t_gripper], True),
                make_fluent_literal(domain_repository, at_robby, [t_room], True),
            ],
            derived_literals=[],
            numeric_constraints=[],
        ),
    )

    drop_effect = get(
        domain_repository,
        ConjunctiveEffectData(
            fluent_literals=[
                make_fluent_literal(domain_repository, at, [t_obj, t_room], True),
                make_fluent_literal(domain_repository, free, [t_gripper], True),
                make_fluent_literal(domain_repository, carry, [t_obj, t_gripper], False),
            ],
            fluent_numeric_effects=[],
            auxiliary_numeric_effect=None,
        ),
    )

    drop_conditional_effect = get(
        domain_repository,
        ConditionalEffectData(
            variables=[],
            condition=get(
                domain_repository,
                ConjunctiveConditionData(
                    variables=[],
                    static_literals=[],
                    fluent_literals=[],
                    derived_literals=[],
                    numeric_constraints=[],
                ),
            ),
            effect=drop_effect,
        ),
    )

    drop = get(
        domain_repository,
        ActionData(
            name="drop",
            original_arity=3,
            variables=[v_obj, v_room, v_gripper],
            condition=drop_condition,
            effects=[drop_conditional_effect],
        ),
    )

    domain = get(
        domain_repository,
        DomainData(
            name="gripper-strips",
            static_predicates=[room, ball, gripper],
            fluent_predicates=[at_robby, at, free, carry],
            derived_predicates=[],
            static_functions=[],
            fluent_functions=[],
            auxiliary_function=None,
            constants=[rooma, roomb],
            actions=[move, pick, drop],
            axioms=[],
        ),
    )

    print(domain)
    print()

    # --------------------------------------------------------------------------
    # 2. Build the lifted task
    # --------------------------------------------------------------------------

    # Create a child repository, effectively inheriting all domain structures.
    # 
    task_repository = factory.create_repository(domain_repository)

    fdr_context = FDRContext(task_repository)

    left = get(task_repository, ObjectData("left"))
    right = get(task_repository, ObjectData("right"))
    ball1 = get(task_repository, ObjectData("ball1"))

    # Static atoms from typing
    static_atoms = [
        make_static_ground_atom(task_repository, room, [rooma]),
        make_static_ground_atom(task_repository, room, [roomb]),
        make_static_ground_atom(task_repository, gripper, [left]),
        make_static_ground_atom(task_repository, gripper, [right]),
        make_static_ground_atom(task_repository, ball, [ball1]),
    ]

    # Initial fluent atoms
    fluent_atoms = [
        make_fluent_ground_atom(task_repository, free, [left]),
        make_fluent_ground_atom(task_repository, free, [right]),
        make_fluent_ground_atom(task_repository, at, [ball1, rooma]),
        make_fluent_ground_atom(task_repository, at_robby, [rooma]),
    ]

    # --------------------------------------------------------------------------
    # Goal
    #
    # Your GroundConjunctiveConditionData expects fluent goals as FDR facts,
    # not as fluent ground atoms.
    #
    # We use the FDR context to automatically create binary FDR variables
    # But we could also initialize the FDRContext with other disjoint mutexes.
    # --------------------------------------------------------------------------

    at_ball1_rooma = make_fluent_ground_atom(task_repository, at, [ball1, rooma])
    at_ball1_roomb = make_fluent_ground_atom(task_repository, at, [ball1, roomb])

    goal_at_ball1_roomb= fdr_context.get_fact(at_ball1_roomb)

    goal = get(
        task_repository,
        GroundConjunctiveConditionData(
            static_literals=[],            
            derived_literals=[],
            positive_facts=[goal_at_ball1_roomb],
            negative_facts=[],
            numeric_constraints=[],
        ),
    )

    task = get(
        task_repository,
        LiftedTaskData(
            name="gripper-1",
            domain=domain,
            derived_predicates=[],
            objects=[left, right, ball1],
            static_atoms=static_atoms,
            fluent_atoms=fluent_atoms,
            static_fterm_values=[],
            fluent_fterm_values=[],
            auxiliary_fterm_value=None,
            goal=goal,
            metric=None,
            axioms=[],
        ),
    )

    print(task)


    # --------------------------------------------------------------------------
    # 3. Group the planning structures and instantiate a ground task
    #
    # We wrap the lifted domain and task together with their repositories in the
    # higher-level planning interface. This allows us to construct a search task
    # and instantiate the fully grounded representation used for search.
    # --------------------------------------------------------------------------

    # Combine the lifted domain with its repository and factory.
    planning_domain = PlanningDomain(domain, domain_repository, factory)

    # Combine the lifted task with the FDR context, task repository, and domain.
    planning_task = PlanningTask(task, fdr_context, task_repository, planning_domain)

    # Create a search task from the planning task.
    search_task = Task(planning_task)

    # Instantiate the fully grounded task representation.
    ground_task_instantiation_result = search_task.instantiate_ground_task(ExecutionContext(1), GroundTaskInstantiationOptions())
    ground_search_task = ground_task_instantiation_result.task

    # Print the grounded formalism task.
    print(ground_search_task.get_formalism_task().get_task())



if __name__ == "__main__":
    main()