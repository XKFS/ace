#include "script_component.hpp"

#include <engine/meta/ecs/entity.hpp>
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
namespace
{
template<typename T>
auto is_supported_type(const mono::mono_type& type) -> bool
{
    const auto& expected_name = type.get_fullname();
    bool is_supported = std::is_same_v<entt::entity, T> && expected_name == "Ace.Core.Entity";

    if(!is_supported)
    {
        is_supported |= mono::is_compatible_type<T>(type);
    }

    return is_supported;
}
} // namespace
REFLECT(script_component)
{
    rttr::registration::class_<script_component>("script_component")(rttr::metadata("category", "SCRIPTING"),
                                                                     rttr::metadata("pretty_name", "Script"))
        .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);
}

template<typename Archive, typename T>
struct mono_saver
{
    static auto try_save_mono_field(ser20::detail::OutputArchiveBase& arbase,
                                    const mono::mono_object& obj,
                                    const mono::mono_field& field) -> bool
    {
        auto& ar = static_cast<Archive&>(arbase);
        auto mutable_field = mono::make_field_invoker<T>(field);
        auto val = mutable_field.get_value(obj);
        return try_save(ar, ser20::make_nvp(field.get_name(), val));
    }

    static auto try_save_mono_property(ser20::detail::OutputArchiveBase& arbase,
                                       const mono::mono_object& obj,
                                       const mono::mono_property& prop) -> bool
    {
        auto& ar = static_cast<Archive&>(arbase);
        auto mutable_prop = mono::make_property_invoker<T>(prop);
        auto val = mutable_prop.get_value(obj);
        return try_save(ar, ser20::make_nvp(prop.get_name(), val));
    }
};

template<typename Archive>
struct mono_saver<Archive, entt::entity>
{
    static auto try_save_mono_field(ser20::detail::OutputArchiveBase& arbase,
                                    const mono::mono_object& obj,
                                    const mono::mono_field& field) -> bool
    {
        auto& ar = static_cast<Archive&>(arbase);

        entt::handle e;

        if(!is_saving_single())
        {
            auto mutable_field = mono::make_field_invoker<entt::entity>(field);
            auto val = mutable_field.get_value(obj);

            auto& ctx = engine::context();
            auto& ec = ctx.get<ecs>();
            auto& scene = ec.get_scene();
            e = scene.create_entity(val);
        }

        return try_save(ar, ser20::make_nvp(field.get_name(), e));
    }

    static auto try_save_mono_property(ser20::detail::OutputArchiveBase& arbase,
                                       const mono::mono_object& obj,
                                       const mono::mono_property& prop) -> bool
    {
        auto& ar = static_cast<Archive&>(arbase);

        entt::handle e;

        if(!is_saving_single())
        {
            auto mutable_prop = mono::make_property_invoker<entt::entity>(prop);
            auto val = mutable_prop.get_value(obj);

            auto& ctx = engine::context();
            auto& ec = ctx.get<ecs>();
            auto& scene = ec.get_scene();
            e = scene.create_entity(val);
        }

        return try_save(ar, ser20::make_nvp(prop.get_name(), e));
    }
};

template<typename Archive, typename T>
struct mono_loader
{
    static auto try_load_mono_field(ser20::detail::InputArchiveBase& arbase,
                                    mono::mono_object& obj,
                                    mono::mono_field& field) -> bool
    {
        auto& ar = static_cast<Archive&>(arbase);

        if(is_supported_type<T>(field.get_type()))
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

    static auto try_load_mono_property(ser20::detail::InputArchiveBase& arbase,
                                       mono::mono_object& obj,
                                       mono::mono_property& prop) -> bool
    {
        auto& ar = static_cast<Archive&>(arbase);

        if(is_supported_type<T>(prop.get_type()))
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
};

template<typename Archive>
struct mono_loader<Archive, entt::entity>
{
    static auto try_load_mono_field(ser20::detail::InputArchiveBase& arbase,
                                    mono::mono_object& obj,
                                    mono::mono_field& field) -> bool
    {
        auto& ar = static_cast<Archive&>(arbase);

        if(is_supported_type<entt::entity>(field.get_type()))
        {
            auto mutable_field = mono::make_field_invoker<entt::entity>(field);
            entt::handle val{};
            if(try_load(ar, ser20::make_nvp(field.get_name(), val)))
            {
                mutable_field.set_value(obj, val.entity());
            }
            return true;
        }
        return false;
    }

    static auto try_load_mono_property(ser20::detail::InputArchiveBase& arbase,
                                       mono::mono_object& obj,
                                       mono::mono_property& prop) -> bool
    {
        auto& ar = static_cast<Archive&>(arbase);

        if(is_supported_type<entt::entity>(prop.get_type()))
        {
            auto mutable_prop = mono::make_property_invoker<entt::entity>(prop);
            entt::handle val{};
            if(try_load(ar, ser20::make_nvp(prop.get_name(), val)))
            {
                mutable_prop.set_value(obj, val.entity());
            }
            return true;
        }
        return false;
    }
};

SAVE(script_component::script_object)
{
    using mono_field_serializer =
        std::function<bool(ser20::detail::OutputArchiveBase&, const mono::mono_object&, const mono::mono_field&)>;

    auto get_field_serilizer = [](const std::string& type_name) -> const mono_field_serializer&
    {
        // clang-format off
        static std::map<std::string, mono_field_serializer> reg = {
            {"SByte",   &mono_saver<Archive, int8_t>::try_save_mono_field},
            {"Byte",    &mono_saver<Archive, uint8_t>::try_save_mono_field},
            {"Int16",   &mono_saver<Archive, int16_t>::try_save_mono_field},
            {"UInt16",  &mono_saver<Archive, uint16_t>::try_save_mono_field},
            {"Int32",   &mono_saver<Archive, int32_t>::try_save_mono_field},
            {"UInt32",  &mono_saver<Archive, uint32_t>::try_save_mono_field},
            {"Int64",   &mono_saver<Archive, int64_t>::try_save_mono_field},
            {"UInt64",  &mono_saver<Archive, uint64_t>::try_save_mono_field},
            {"Boolean", &mono_saver<Archive, bool>::try_save_mono_field},
            {"Single",  &mono_saver<Archive, float>::try_save_mono_field},
            {"Double",  &mono_saver<Archive, double>::try_save_mono_field},
            {"Char",    &mono_saver<Archive, char16_t>::try_save_mono_field},
            {"String",  &mono_saver<Archive, std::string>::try_save_mono_field},
            {"Entity",  &mono_saver<Archive, entt::entity>::try_save_mono_field}
        };
        // clang-format on

        auto it = reg.find(type_name);
        if(it != reg.end())
        {
            return it->second;
        }
        static const mono_field_serializer empty;
        return empty;
    };

    using mono_property_serializer =
        std::function<bool(ser20::detail::OutputArchiveBase&, const mono::mono_object&, const mono::mono_property&)>;

    auto get_property_serilizer = [](const std::string& type_name) -> const mono_property_serializer&
    {
        // clang-format off
        static std::map<std::string, mono_property_serializer> reg = {
            {"SByte",   &mono_saver<Archive, int8_t>::try_save_mono_property},
            {"Byte",    &mono_saver<Archive, uint8_t>::try_save_mono_property},
            {"Int16",   &mono_saver<Archive, int16_t>::try_save_mono_property},
            {"UInt16",  &mono_saver<Archive, uint16_t>::try_save_mono_property},
            {"Int32",   &mono_saver<Archive, int32_t>::try_save_mono_property},
            {"UInt32",  &mono_saver<Archive, uint32_t>::try_save_mono_property},
            {"Int64",   &mono_saver<Archive, int64_t>::try_save_mono_property},
            {"UInt64",  &mono_saver<Archive, uint64_t>::try_save_mono_property},
            {"Boolean", &mono_saver<Archive, bool>::try_save_mono_property},
            {"Single",  &mono_saver<Archive, float>::try_save_mono_property},
            {"Double",  &mono_saver<Archive, double>::try_save_mono_property},
            {"Char",    &mono_saver<Archive, char16_t>::try_save_mono_property},
            {"String",  &mono_saver<Archive, std::string>::try_save_mono_property}
        };
        // clang-format on

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
        std::function<bool(ser20::detail::InputArchiveBase&, mono::mono_object&, mono::mono_field&)>;

    auto get_field_serilizer = [](const std::string& type_name) -> const mono_field_serializer&
    {
        // clang-format off
        static std::map<std::string, mono_field_serializer> reg = {
            {"SByte",   &mono_loader<Archive, int8_t>::try_load_mono_field},
            {"Byte",    &mono_loader<Archive, uint8_t>::try_load_mono_field},
            {"Int16",   &mono_loader<Archive, int16_t>::try_load_mono_field},
            {"UInt16",  &mono_loader<Archive, uint16_t>::try_load_mono_field},
            {"Int32",   &mono_loader<Archive, int32_t>::try_load_mono_field},
            {"UInt32",  &mono_loader<Archive, uint32_t>::try_load_mono_field},
            {"Int64",   &mono_loader<Archive, int64_t>::try_load_mono_field},
            {"UInt64",  &mono_loader<Archive, uint64_t>::try_load_mono_field},
            {"Boolean", &mono_loader<Archive, bool>::try_load_mono_field},
            {"Single",  &mono_loader<Archive, float>::try_load_mono_field},
            {"Double",  &mono_loader<Archive, double>::try_load_mono_field},
            {"Char",    &mono_loader<Archive, char16_t>::try_load_mono_field},
            {"String",  &mono_loader<Archive, std::string>::try_load_mono_field},
            {"Entity",  &mono_loader<Archive, entt::entity>::try_load_mono_field}
        };
        // clang-format on

        auto it = reg.find(type_name);
        if(it != reg.end())
        {
            return it->second;
        }
        static const mono_field_serializer empty;
        return empty;
    };

    using mono_property_serializer =
        std::function<bool(ser20::detail::InputArchiveBase&, mono::mono_object&, mono::mono_property&)>;

    auto get_property_serilizer = [](const std::string& type_name) -> const mono_property_serializer&
    {
        // clang-format off
        static std::map<std::string, mono_property_serializer> reg = {
            {"SByte",   &mono_loader<Archive, int8_t>::try_load_mono_property},
            {"Byte",    &mono_loader<Archive, uint8_t>::try_load_mono_property},
            {"Int16",   &mono_loader<Archive, int16_t>::try_load_mono_property},
            {"UInt16",  &mono_loader<Archive, uint16_t>::try_load_mono_property},
            {"Int32",   &mono_loader<Archive, int32_t>::try_load_mono_property},
            {"UInt32",  &mono_loader<Archive, uint32_t>::try_load_mono_property},
            {"Int64",   &mono_loader<Archive, int64_t>::try_load_mono_property},
            {"UInt64",  &mono_loader<Archive, uint64_t>::try_load_mono_property},
            {"Boolean", &mono_loader<Archive, bool>::try_load_mono_property},
            {"Single",  &mono_loader<Archive, float>::try_load_mono_property},
            {"Double",  &mono_loader<Archive, double>::try_load_mono_property},
            {"Char",    &mono_loader<Archive, char16_t>::try_load_mono_property},
            {"String",  &mono_loader<Archive, std::string>::try_load_mono_property}
        };
        // clang-format on

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
        if(comp.scoped)
        {
            obj.add_script_component(comp);
        }
    }
}
LOAD_INSTANTIATE(script_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(script_component, ser20::iarchive_binary_t);
} // namespace ace
