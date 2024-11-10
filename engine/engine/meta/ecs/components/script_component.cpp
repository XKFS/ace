#include "script_component.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include <engine/engine.h>
#include <engine/events.h>

#include <engine/scripting/ecs/systems/script_system.h>
#include <monopp/mono_field_invoker.h>
#include <monopp/mono_property_invoker.h>
#include <monopp/mono_type.h>

namespace ace
{
REFLECT(script_component)
{
    rttr::registration::class_<script_component>("script_component")(rttr::metadata("category", "SCRIPTING"),
                                                                     rttr::metadata("pretty_name", "Script"))
        .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);
}

template<typename Archive, typename T>
auto try_save_mono_field(ser20::detail::OutputArchiveBase& arbase,
                         const mono::mono_object& obj,
                         const mono::mono_field& field) -> bool
{
    auto& ar = static_cast<Archive&>(arbase);
    auto mutable_field = mono::make_field_invoker<T>(field);
    auto val = mutable_field.get_value(obj);
    return try_save(ar, ser20::make_nvp(field.get_name(), val));
}

template<typename Archive, typename T>
auto try_save_mono_property(ser20::detail::OutputArchiveBase& arbase,
                            const mono::mono_object& obj,
                            const mono::mono_property& prop) -> bool
{
    auto& ar = static_cast<Archive&>(arbase);
    auto mutable_field = mono::make_property_invoker<T>(prop);
    auto val = mutable_field.get_value(obj);
    return try_save(ar, ser20::make_nvp(prop.get_name(), val));
}

template<typename Archive, typename T>
auto try_load_mono_field(ser20::detail::InputArchiveBase& arbase, mono::mono_object& obj, mono::mono_field& field)
    -> bool
{
    auto& ar = static_cast<Archive&>(arbase);

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

template<typename Archive, typename T>
auto try_load_mono_property(ser20::detail::InputArchiveBase& arbase,
                            mono::mono_object& obj,
                            mono::mono_property& prop) -> bool
{
    auto& ar = static_cast<Archive&>(arbase);

    if(mono::is_compatible_type<T>(prop.get_type()))
    {
        auto mutable_field = mono::make_property_invoker<T>(prop);
        T val{};
        if(try_load(ar, ser20::make_nvp(prop.get_name(), val)))
        {
            mutable_field.set_value(obj, val);
        }
        return true;
    }
    return false;
}

SAVE(script_component::script_object)
{
    using mono_field_serializer =
        std::function<void(ser20::detail::OutputArchiveBase&, const mono::mono_object&, const mono::mono_field&)>;

    auto get_field_serilizer = [](const std::string& type_name) -> const mono_field_serializer&
    {
        static std::map<std::string, mono_field_serializer> reg = {
            {"SByte", &try_save_mono_field<Archive, int8_t>},
            {"Byte", &try_save_mono_field<Archive, uint8_t>},
            {"Int16", &try_save_mono_field<Archive, int16_t>},
            {"UInt16", &try_save_mono_field<Archive, uint16_t>},
            {"Int32", &try_save_mono_field<Archive, int32_t>},
            {"UInt32", &try_save_mono_field<Archive, uint32_t>},
            {"Int64", &try_save_mono_field<Archive, int64_t>},
            {"UInt64", &try_save_mono_field<Archive, uint64_t>},
            {"Boolean", &try_save_mono_field<Archive, bool>},
            {"Single", &try_save_mono_field<Archive, float>},
            {"Double", &try_save_mono_field<Archive, double>},
            {"Char", &try_save_mono_field<Archive, char16_t>},
            {"String", &try_save_mono_field<Archive, std::string>}};

        auto it = reg.find(type_name);
        if(it != reg.end())
        {
            return it->second;
        }
        static const mono_field_serializer empty;
        return empty;
    };

    using mono_property_serializer =
        std::function<void(ser20::detail::OutputArchiveBase&, const mono::mono_object&, const mono::mono_property&)>;

    auto get_property_serilizer = [](const std::string& type_name) -> const mono_property_serializer&
    {
        static std::map<std::string, mono_property_serializer> reg = {
            {"SByte", &try_save_mono_property<Archive, int8_t>},
            {"Byte", &try_save_mono_property<Archive, uint8_t>},
            {"Int16", &try_save_mono_property<Archive, int16_t>},
            {"UInt16", &try_save_mono_property<Archive, uint16_t>},
            {"Int32", &try_save_mono_property<Archive, int32_t>},
            {"UInt32", &try_save_mono_property<Archive, uint32_t>},
            {"Int64", &try_save_mono_property<Archive, int64_t>},
            {"UInt64", &try_save_mono_property<Archive, uint64_t>},
            {"Boolean", &try_save_mono_property<Archive, bool>},
            {"Single", &try_save_mono_property<Archive, float>},
            {"Double", &try_save_mono_property<Archive, double>},
            {"Char", &try_save_mono_property<Archive, char16_t>},
            {"String", &try_save_mono_property<Archive, std::string>}};

        auto it = reg.find(type_name);
        if(it != reg.end())
        {
            return it->second;
        }
        static const mono_property_serializer empty;
        return empty;
    };

    const auto& object = obj.scoped->object;
    const auto& type = object.get_type();

    try_save(ar, ser20::make_nvp("type", type.get_fullname()));

    auto fields = type.get_fields();
    for(auto& field : fields)
    {
        if(field.get_visibility() == mono::visibility::vis_public)
        {
            const auto& field_type = field.get_type();

            auto field_serilizer = get_field_serilizer(field_type.get_name());
            if(field_serilizer)
            {
                field_serilizer(ar, object, field);
            }
        }
    }

    auto properties = type.get_properties();
    for(auto& prop : properties)
    {
        if(prop.get_visibility() == mono::visibility::vis_public)
        {
            const auto& prop_type = prop.get_type();

            auto prop_serilizer = get_property_serilizer(prop_type.get_name());
            if(prop_serilizer)
            {
                prop_serilizer(ar, object, prop);
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

    using mono_field_serializer =
        std::function<void(ser20::detail::InputArchiveBase&, mono::mono_object&, mono::mono_field&)>;

    auto get_field_serilizer = [](const std::string& type_name) -> const mono_field_serializer&
    {
        static std::map<std::string, mono_field_serializer> reg = {
            {"SByte", &try_load_mono_field<Archive, int8_t>},
            {"Byte", &try_load_mono_field<Archive, uint8_t>},
            {"Int16", &try_load_mono_field<Archive, int16_t>},
            {"UInt16", &try_load_mono_field<Archive, uint16_t>},
            {"Int32", &try_load_mono_field<Archive, int32_t>},
            {"UInt32", &try_load_mono_field<Archive, uint32_t>},
            {"Int64", &try_load_mono_field<Archive, int64_t>},
            {"UInt64", &try_load_mono_field<Archive, uint64_t>},
            {"Boolean", &try_load_mono_field<Archive, bool>},
            {"Single", &try_load_mono_field<Archive, float>},
            {"Double", &try_load_mono_field<Archive, double>},
            {"Char", &try_load_mono_field<Archive, char16_t>},
            {"String", &try_load_mono_field<Archive, std::string>}};

        auto it = reg.find(type_name);
        if(it != reg.end())
        {
            return it->second;
        }
        static const mono_field_serializer empty;
        return empty;
    };

    using mono_property_serializer =
        std::function<void(ser20::detail::InputArchiveBase&, mono::mono_object&, mono::mono_property&)>;

    auto get_property_serilizer = [](const std::string& type_name) -> const mono_property_serializer&
    {
        static std::map<std::string, mono_property_serializer> reg = {
            {"SByte", &try_load_mono_property<Archive, int8_t>},
            {"Byte", &try_load_mono_property<Archive, uint8_t>},
            {"Int16", &try_load_mono_property<Archive, int16_t>},
            {"UInt16", &try_load_mono_property<Archive, uint16_t>},
            {"Int32", &try_load_mono_property<Archive, int32_t>},
            {"UInt32", &try_load_mono_property<Archive, uint32_t>},
            {"Int64", &try_load_mono_property<Archive, int64_t>},
            {"UInt64", &try_load_mono_property<Archive, uint64_t>},
            {"Boolean", &try_load_mono_property<Archive, bool>},
            {"Single", &try_load_mono_property<Archive, float>},
            {"Double", &try_load_mono_property<Archive, double>},
            {"Char", &try_load_mono_property<Archive, char16_t>},
            {"String", &try_load_mono_property<Archive, std::string>}};

        auto it = reg.find(type_name);
        if(it != reg.end())
        {
            return it->second;
        }
        static const mono_property_serializer empty;
        return empty;
    };

    auto fields = script_type.get_fields();
    for(auto& field : fields)
    {
        if(field.get_visibility() == mono::visibility::vis_public)
        {
            const auto& field_type = field.get_type();

            auto field_serilizer = get_field_serilizer(field_type.get_name());
            if(field_serilizer)
            {
                field_serilizer(ar, object, field);
            }
        }
    }

    // auto properties = script_type.get_properties();
    // for(auto& prop : properties)
    // {
    //     if(prop.get_visibility() == mono::visibility::vis_public)
    //     {
    //         const auto& prop_type = prop.get_type();

    //         auto prop_serilizer = get_property_serilizer(prop_type.get_name());
    //         if(prop_serilizer)
    //         {
    //             prop_serilizer(ar, object, prop);
    //         }
    //     }
    // }
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
