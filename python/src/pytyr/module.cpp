#include "module.hpp"

#include "formalism/module.hpp"
#include "planning/module.hpp"

namespace tyr
{

void bind_module_definitions(nb::module_& m)
{
    auto formalism_module = m.def_submodule("formalism");
    ::tyr::formalism::bind_module_definitions(formalism_module);

    auto planning_module = m.def_submodule("planning");
    planning::bind_module_definitions(planning_module);
}

}  // namespace tyr
