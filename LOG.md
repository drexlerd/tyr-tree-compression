# Common Extraction Notes

`include/tyr/common` is still part of Tyr. The current cleanup direction is to make it internally consistent and easier to extract later, without creating a standalone target, repository, or package yet.

## Current Responsibilities

`common` currently contains several different categories of helpers:

- Core scalar/config aliases: `int_t`, `uint_t`, `float_t`, float tolerance policy, and cista serialization mode.
- Generic comparison utilities: `Hash`, `EqualTo`, `Less`, `hash_range`, `equal_range`, and `less_range`.
- Storage and container utilities: segmented vectors, indexed hash sets, raw pools/sets, block array pools/sets, bit-packed pools/sets, and object pools.
- Generic view/adaptor infrastructure: `Data`, `Index`, `View`, `Builder`, and cista-backed vector/optional/variant view adapters.
- Utility headers: bit manipulation, chrono helpers, intervals, dynamic bitsets, path/file helpers, JSON loading/accessors, formatting, memory, observer pointers, and TBB parallelism wrappers.
- Integration helpers: fmt formatters, nanobind type casters, Boost.JSON helpers, Boost interval/dynamic bitset utilities, cista adapters, gtl aliases, and TBB wrappers.

## Current Dependency Shape

`common` does not directly include planner, search, datalog, formalism, or analysis headers. The remaining coupling is mostly conceptual and third-party oriented:

- `cista` is embedded in core aliases, view adapters, serialization mode, hashing, equality, formatting, and Python type casters.
- `gtl` is embedded through `flat_hash_*`, `btree_*`, `phmap_mix`, and memory accounting with `gtl::priv::ctrl_t`.
- Boost is embedded through JSON, interval, and dynamic bitset helpers.
- fmt, nanobind, and TBB integrations live under `common` and are pulled by some broad headers.
- `Data`, `Index`, `Builder`, and `View` are generic names, but their current usage is dominated by Tyr repository/formalism/planning storage patterns.

## Recent Boundary Improvements

The following boundaries are now more explicit:

- Reusable concepts now live in `concepts.hpp` and are re-exported by the lightweight `core.hpp` umbrella. Reusable cv-ref-normalized and const-normalized template constraints use `SameAsIgnoringCvref` and `UnsignedIntegralSameAsIgnoringConst`; repository context constraints also expose `CanonicalizableContextFor` so concept annotations stay in the `template<...>` head where possible. Raw pools, pool-backed sets, view infrastructure, closed interval helpers, generic hash/equality helpers, comparators, formatters, and associative container aliases include lighter direct dependencies instead of the heavier declaration aliases where possible.
- A lightweight opt-in umbrella now lives in `core.hpp`.
- Associative container aliases now live in `associative_containers.hpp` and are re-exported by `containers.hpp` and the legacy `declarations.hpp` compatibility shim.
- Heavier pool and storage helpers are grouped by `containers.hpp`.
- Reusable third-party adapters are grouped by `adapters.hpp`. The TBB execution context now validates requested thread counts before initializing its task arena.
- Tyr build-tree adapters are grouped by `project_adapters.hpp`, which includes `project_path.hpp` and `json_suite.hpp`.
- Repository/view infrastructure is grouped by `repository_types.hpp`.
- Generic path/file helpers live in `path.hpp`.
- Checked conversion to the common `uint_t` index type lives in `config.hpp` and is used by common index-producing containers before narrowing sizes. Direct users now increasingly include `associative_containers.hpp`, `concepts.hpp`, `config.hpp`, `types.hpp`, `hash.hpp`, and `equal_to.hpp` directly instead of the broad declaration shim when they only need aliases, concepts, primitive config, repository type declarations, or concrete gtl containers.
- `ROOT_DIR` project convenience helpers live in `project_path.hpp` and are intentionally outside `adapters.hpp`.
- JSON suite path helpers live in `json_suite.hpp`.
- Generic JSON file loading, required/optional member lookup, and typed JSON accessors live in `json.hpp`; optional typed field readers for objects, arrays, strings, booleans, sizes, `uint_t` values, and doubles return `std::optional`.
- `json_loader.hpp` remains as a compatibility umbrella for generic JSON helpers plus Tyr JSON-suite helpers.
- Range hashing/equality/ordering helpers are centralized in `hash.hpp`, `equal_to.hpp`, and `comparators.hpp`.
- Canonicalization of `DataList` and `IndexList` now shares one list sort/unique implementation, and formatter convenience helpers have focused common tests.
- Raw/block/bit-packed set internals reuse the shared range helpers instead of local range loops, including raw vector set lookup over range-like raw vector views.
- Raw array and vector sets now expose `contains` and `empty` like the block and bit-packed set containers. `IndexedHashSet` now exposes `contains` as well, and block/bit-packed/indexed sets expose `contains_with_hash` where they already expose hash-aware lookup; block and bit-packed set insertion paths share unchecked internals after public validation, matching the set-like lookup surface. Their lookup and insertion APIs accept `std::span<const T>`, so callers can use any contiguous storage without materializing `std::vector`. Bit-packed set tests cover transparent lookup plus stored value access. Raw array/vector pools, raw array/vector sets, raw vector views, block array views, and bit-packed array views now expose `empty()` as well; raw pools and sets also expose `front()` like the other pool-backed set containers. They also expose `back()` where the underlying storage has stable indexed order. Raw vector views are range-like via `begin()`/`end()`, so callers can iterate without spelling `data()` plus `size()`.
- `SegmentedVector` now has focused common tests for empty state, segmented indexing, mutable `front`/`back`/`at`, `pop_back`, `clear`, and out-of-range access. This documents the standalone container behavior without changing the API.
- Unique and shared object pools now expose `size()`, `empty()`, and `free_size()` alongside compatibility `get_size()` and `get_num_free()` wrappers, matching the dominant common container naming style.

`json_loader.hpp` intentionally includes the split path/suite headers for source compatibility with current test and profiling users.


## Data/Index/View Ownership Audit

`Data`, `Index`, `Builder`, and `View` are currently declared in `common/types.hpp`, but their dominant ownership is Tyr repository modeling rather than generic utilities. Evidence from the current tree:

- `Data<T>` specializations live primarily in formalism, datalog, planning, and match-tree data headers.
- `Index<T>` specializations live primarily in corresponding domain index headers and usually derive from `IndexMixin`.
- `View<T, C>` specializations encode repository-backed access patterns: they store a handle plus a context/repository and expose `get_data`, `get_context`, and `get_handle`.
- `DataList<T>` and `IndexList<T>` are cista offset vectors, so the list aliases couple the generic names directly to cista storage.
- `types_utils.hpp` contains convenience mutation helpers for `View<Index<T>, C>`, `View<Data<T>, C>`, `IndexList<T>`, and `DataList<T>`, which makes it repository-adapter code rather than purely generic type utilities.

Recommended direction: keep these declarations stable for now, but treat them as a separate repository/view infrastructure layer, not part of the smallest extractable helper core. The non-breaking code step now exists as `repository_types.hpp`, which includes `types.hpp`, `types_utils.hpp`, `index_mixins.hpp`, and the cista-backed view adapters. Existing includes can migrate gradually to that umbrella before any namespace or directory move is considered.

## Extraction Blockers

1. `Data`, `Index`, `Builder`, and `View` need caller migration to the repository/view infrastructure boundary.
   The audit above shows they are foundational for Tyr data modeling, but too repository-shaped for the smallest extractable helper core. `repository_types.hpp` now provides a compatibility umbrella; direct users can migrate to it before considering relocation.

2. The legacy umbrella header `common.hpp` remains a compatibility umbrella.
   `core.hpp`, `containers.hpp`, and `adapters.hpp` now provide smaller opt-in boundaries, but future extraction still needs target-level dependency separation before introducing a library target.

3. `project_path.hpp` is build-tree specific.
   Its helpers depend on `ROOT_DIR`. They are now grouped under `project_adapters.hpp`, but extraction still needs a decision to keep that adapter in Tyr, make it optional, or replace it with caller-provided roots.

4. `json_suite.hpp` is test/profiling-suite shaped.
   It is isolated from generic JSON accessors and grouped under `project_adapters.hpp` now, but its `prefix` convention is a Tyr test/profiling convention rather than a generally reusable JSON helper.

5. Third-party dependencies are not separated by adapter boundary.
   cista, gtl, Boost, fmt, TBB, and nanobind support are useful, but a standalone helper library should not require all of them for basic path, hash, equality, or bit utilities.

6. Pool-backed set containers still repeat an indexed-storage pattern.
   `IndexedHashSet`, `BlockArraySet`, `BitPackedArraySet`, `RawArraySet`, and `RawVectorSet` share transparent lookup and pool-backed index storage patterns that could be factored after their tests are strong enough.

7. Error and precondition policy remains mixed.
   Public runtime validation now increasingly throws typed exceptions, including raw and block-array length checks plus raw vector size-limit checks; remaining low-level indexing and internal hash-table invariants still use `assert`, and JSON/path helpers throw `std::runtime_error`. Extraction should document or normalize this policy per helper category.

## Prioritized Cleanup Backlog

1. Continue moving callers from `common.hpp` and `tyr.hpp` to smaller umbrella headers.
   `common/core.hpp`, `common/containers.hpp`, `common/adapters.hpp`, and `common/json.hpp` exist now. The direct C++ unit-test use of `tyr/tyr.hpp` has been narrowed; the remaining top-level umbrella users are Python binding files and should be migrated only with a bindings-enabled validation build.

2. Migrate direct users of repository/view infrastructure to `repository_types.hpp`.
   Prefer the named boundary over piecemeal includes of `types.hpp`, `types_utils.hpp`, `index_mixins.hpp`, and cista-backed view adapters when callers need the full repository/view layer.

3. Separate third-party adapters at target/dependency level.
   Header grouping exists now, but cista, gtl, Boost, fmt, TBB, and nanobind integrations still need dependency-level boundaries before extraction.

4. Consolidate pool-backed set internals.
   Factor only after the existing raw/block/bit-packed/indexed set tests cover insertion, duplicate detection, lookup, clear, and stored value access.

5. Normalize exception and precondition behavior.
   Document which helpers assert caller preconditions and which validate runtime input with exceptions.

6. Continue replacing direct Boost.JSON lookup in tests and profiling code with `json_loader.hpp` member accessors.
   Use `find_member`, `find_object`, `find_array`, `find_string`, `find_size`, `find_uint_t`, `find_double`, `require_member`, and typed object-key accessors for fixture parsing so callers rely less on Boost.JSON mechanics.

7. Decide the final home for Tyr-specific project adapters.
   `project_adapters.hpp` marks `project_path.hpp` and `json_suite.hpp` as a separate boundary, but those headers still need to stay out of the smallest extractable helper core or become optional.
