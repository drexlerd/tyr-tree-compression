#ifndef TYR_FORMALISM_BINDING_INDEX_HPP_
#define TYR_FORMALISM_BINDING_INDEX_HPP_

#include <yggdrasil/formalism/binding_index.hpp>
#include "tyr/formalism/declarations.hpp"

namespace tyr::formalism
{
template<typename Tag, std::ranges::forward_range BindingRange>
    requires std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<BindingRange>>, ygg::Index<Row>>
using RelationBindingsForwardRange = ::ygg::formalism::RelationBindingsForwardRange<Tag, ObjectTag, BindingRange>;

template<typename Tag, std::ranges::random_access_range BindingRange>
    requires std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<BindingRange>>, ygg::Index<Row>>
using RelationBindingsRandomAccessRange = ::ygg::formalism::RelationBindingsRandomAccessRange<Tag, ObjectTag, BindingRange>;
}

#endif
