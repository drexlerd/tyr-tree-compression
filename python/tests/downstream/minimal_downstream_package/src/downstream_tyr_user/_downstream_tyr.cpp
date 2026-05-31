#include <nanobind/nanobind.h>

#include <yggdrasil/core/config.hpp>
#include <tyr/formalism/planning/repository.hpp>

namespace nb = nanobind;

namespace
{
[[maybe_unused]] [[gnu::used]] void link_planning_view_methods(tyr::formalism::planning::TaskView task,
                                                               tyr::formalism::planning::FDRFactView<tyr::formalism::FluentTag> fact)
{
    static_cast<void>(task.get_goal());
    static_cast<void>(fact.get_atom());
}
}

NB_MODULE(_downstream_tyr, m)
{
    m.def("float_t_size", [] { return sizeof(ygg::float_t); });
    m.def("multiply", [](int lhs, int rhs) { return lhs * rhs; });
}
