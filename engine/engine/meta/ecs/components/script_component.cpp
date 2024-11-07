#include "script_component.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(script_component)
{
    rttr::registration::class_<script_component>("script_component")(
        rttr::metadata("category", "SCRIPTING"),
        rttr::metadata("pretty_name", "Script"))
        .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);
}

SAVE(script_component)
{
    // const auto& comps = obj.get_script_components();
    // for(const auto& comp : comps)
    // {
    //     comp.
    // }
}
SAVE_INSTANTIATE(script_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(script_component, ser20::oarchive_binary_t);

LOAD(script_component)
{
}
LOAD_INSTANTIATE(script_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(script_component, ser20::iarchive_binary_t);
} // namespace ace
