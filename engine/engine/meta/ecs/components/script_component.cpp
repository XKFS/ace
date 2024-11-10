#include "script_component.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include <engine/engine.h>
#include <engine/events.h>

#include <engine/scripting/ecs/systems/script_system.h>

namespace ace
{
REFLECT(script_component)
{
    rttr::registration::class_<script_component>("script_component")(rttr::metadata("category", "SCRIPTING"),
                                                                     rttr::metadata("pretty_name", "Script"))
        .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);
}

template<typename T, typename Archive>
auto try_save_mono_field(Archive& ar, const mono::mono_object& obj, const mono::mono_field& field) -> bool
{
    if(mono::is_compatible_type<T>(field.get_type()))
    {
        auto mutable_field = mono::make_field_invoker<T>(field);
        auto val = mutable_field.get_value(obj);
        try_save(ar, ser20::make_nvp(field.get_name(), val));

        return true;
    }

    return false;
}

template<typename T, typename Archive>
auto try_load_mono_field(Archive& ar, mono::mono_object& obj, mono::mono_field& field) -> bool
{
    if(mono::is_compatible_type<T>(field.get_type()))
    {
        auto mutable_field = mono::make_field_invoker<T>(field);
        T val{};
        if(try_load(ar, ser20::make_nvp(field.get_name(), val)))
        {
            mutable_field.set_value(obj, val);
        }
        return true;
    }
    return false;
}

SAVE(script_component::script_object)
{
    const auto& object = obj.scoped->object;
    const auto& type = object.get_type();

    try_save(ar, ser20::make_nvp("type", type.get_fullname()));

    auto fields = type.get_fields();
    for(auto& field : fields)
    {
        if(field.get_visibility() == mono::visibility::vis_public)
        {
            const auto& field_type = field.get_type();

            if(try_save_mono_field<int8_t>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<uint8_t>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<int16_t>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<uint16_t>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<int32_t>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<uint32_t>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<int64_t>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<uint64_t>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<bool>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<float>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<double>(ar, object, field))
            {
                continue;
            }
            if(try_save_mono_field<std::string>(ar, object, field))
            {
                continue;
            }
        }
    }
}
SAVE_INSTANTIATE(script_component::script_object, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(script_component::script_object, ser20::oarchive_binary_t);

LOAD(script_component::script_object)
{
    auto& ctx = engine::context();
    auto& sys = ctx.get<script_system>();
    const auto& all_scriptable_components = sys.get_all_scriptable_components();

    std::string type;
    try_load(ar, ser20::make_nvp("type", type));

    auto it = std::find_if(std::begin(all_scriptable_components),
                           std::end(all_scriptable_components),
                           [&](const mono::mono_type& script_type)
                           {
                               return type == script_type.get_fullname();
                           });

    if(it == std::end(all_scriptable_components))
    {
        return;
    }

    const auto& script_type = *it;
    auto object = script_type.new_instance();
    obj.scoped = std::make_shared<mono::mono_scoped_object>(object);


    auto fields = script_type.get_fields();
    for(auto& field : fields)
    {
        if(field.get_visibility() == mono::visibility::vis_public)
        {
            const auto& field_type = field.get_type();

            if(try_load_mono_field<int8_t>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<uint8_t>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<int16_t>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<uint16_t>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<int32_t>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<uint32_t>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<int64_t>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<uint64_t>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<bool>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<float>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<double>(ar, object, field))
            {
                continue;
            }
            if(try_load_mono_field<std::string>(ar, object, field))
            {
                continue;
            }
        }
    }
}
LOAD_INSTANTIATE(script_component::script_object, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(script_component::script_object, ser20::iarchive_binary_t);

SAVE(script_component)
{
    const auto& comps = obj.get_script_components();

    try_save(ar, ser20::make_nvp("script_components", comps));
}
SAVE_INSTANTIATE(script_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(script_component, ser20::oarchive_binary_t);

LOAD(script_component)
{
    auto& ctx = engine::context();
    auto& sys = ctx.get<script_system>();
    const auto& all_scriptable_components = sys.get_all_scriptable_components();

    script_component::script_components_t comps;
    try_load(ar, ser20::make_nvp("script_components", comps));

    for(const auto& comp : comps)
    {
        obj.add_script_component(comp);
    }
}
LOAD_INSTANTIATE(script_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(script_component, ser20::iarchive_binary_t);
} // namespace ace
