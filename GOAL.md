# List of Requirements:
- Do not recompile after every single code change.
- Use 2 cores for compilation.
- Use `std::shared_ptr` only for APIs/results that own or retain lifetime. Non-owning immediate-use APIs may take `T&`/`const T&`. Do not create borrowed `std::shared_ptr` wrappers.

# List of Goals:

1. Read and continue keeping track of recent changes in LOG.md
2. Continue reducing header dependencies with practical splits only. Do not split every class or functor into its own file. Split only where it reduces compile resources, clarifies ownership, or improves include hygiene.
3. Complete common ordering predicate support: for every common type family that defines `Hash` and `EqualTo`, also provide meaningful `Less`, `LessEqual`, `Greater`, and `GreaterEqual` predicates. Prefer one `*_ordering.hpp` per type family over one file per operator.

# Tasks
The agent should work on the highest unchecked task in this list. The agent must not mark tasks as complete.

- [ ] Inventory all common type families with `Hash` and `EqualTo`.
- [ ] Decide ordering semantics for each family, especially `observer_ptr`.
- [ ] Add compact `*_ordering.hpp` headers and update `include/tyr/common/adapters.hpp`.
- [ ] Add focused ordering tests, including `static_assert` concept checks.
- [ ] Validate targeted common tests, self-contained headers, common include boundaries, and `git diff --check`.
