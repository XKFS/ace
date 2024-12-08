#include "inspector_script.h"
#include "inspectors.h"
#include <engine/assets/asset_manager.h>
#include <monopp/mono_field_invoker.h>
#include <monopp/mono_property_invoker.h>


#include <graphics/texture.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/material.h>

#include <engine/animation/animation.h>
#include <engine/audio/audio_clip.h>
#include <engine/ecs/prefab.h>
#include <engine/physics/physics_material.h>

namespace ace
{

auto find_attribute(const std::string& name, const std::vector<mono::mono_object>& attribs) -> mono::mono_object
{
    auto it = std::find_if(std::begin(attribs),
                           std::end(attribs),
                           [&](const mono::mono_object& obj)
                           {
                               return obj.get_type().get_name() == name;
                           });

    if(it != std::end(attribs))
    {
        return *it;
    }

    return {};
}

template<typename T>
struct mono_inspector
{
    template<typename Invoker>
    static auto inspect_invoker(rtti::context& ctx,
                                mono::mono_object& obj,
                                const Invoker& mutable_field,
                                const var_info& info) -> inspect_result
    {
        auto val = mutable_field.get_value(obj);

        inspect_result result;

        auto attribs = mutable_field.get_attributes();
        auto range_attrib = find_attribute("RangeAttribute", attribs);
        auto min_attrib = find_attribute("MinAttribute", attribs);
        auto max_attrib = find_attribute("MaxAttribute", attribs);
        auto step_attrib = find_attribute("StepAttribute", attribs);
        auto tooltip_attrib = find_attribute("TooltipAttribute", attribs);

        std::string tooltip;
        if(tooltip_attrib.valid())
        {
            auto invoker = mono::make_field_invoker<std::string>(tooltip_attrib.get_type(), "tooltip");
            tooltip = invoker.get_value(tooltip_attrib);
        }

        inspector::meta_getter getter = [&](const rttr::variant& name) -> rttr::variant
        {
            if(!name.is_type<std::string>())
            {
                return {};
            }
            const auto& key = name.get_value<std::string>();
            if(key == "min")
            {
                if(min_attrib.valid())
                {
                    auto invoker = mono::make_field_invoker<float>(min_attrib.get_type(), "min");
                    float min_value = invoker.get_value(min_attrib);
                    return min_value;
                }
                if(range_attrib.valid())
                {
                    auto invoker = mono::make_field_invoker<float>(range_attrib.get_type(), "min");
                    float min_value = invoker.get_value(range_attrib);
                    return min_value;
                }
            }

            else if(key == "max")
            {
                if(max_attrib.valid())
                {
                    auto invoker = mono::make_field_invoker<float>(max_attrib.get_type(), "max");
                    float max_value = invoker.get_value(max_attrib);
                    return max_value;
                }
                if(range_attrib.valid())
                {
                    auto invoker = mono::make_field_invoker<float>(range_attrib.get_type(), "max");
                    float max_value = invoker.get_value(range_attrib);
                    return max_value;
                }
            }

            else if(key == "step")
            {
                if(step_attrib.valid())
                {
                    auto invoker = mono::make_field_invoker<float>(step_attrib.get_type(), "step");
                    float value = invoker.get_value(step_attrib);
                    return value;
                }
            }

            return {};
        };

        rttr::variant var = val;

        {
            property_layout layout(mutable_field.get_name(), tooltip);
            result |= inspect_var(ctx, var, info, getter);
        }

        if(result.changed)
        {
            val = var.get_value<T>();
            mutable_field.set_value(obj, val);
        }

        return result;
    }

    static auto inspect_field(rtti::context& ctx, mono::mono_object& obj, mono::mono_field& field, const var_info& info)
        -> inspect_result
    {
        auto invoker = mono::make_field_invoker<T>(field);

        var_info field_info;
        field_info.is_property = true;
        field_info.read_only = info.read_only || field.is_readonly();

        return inspect_invoker(ctx, obj, invoker, field_info);
    }

    static auto inspect_property(rtti::context& ctx,
                                 mono::mono_object& obj,
                                 mono::mono_property& field,
                                 const var_info& info) -> inspect_result
    {
        auto invoker = mono::make_property_invoker<T>(field);

        var_info field_info;
        field_info.is_property = true;
        field_info.read_only = info.read_only || field.is_readonly();

        return inspect_invoker(ctx, obj, invoker, info);
    }
};

template<>
struct mono_inspector<entt::handle>
{
    template<typename Invoker>
    static auto inspect_invoker(rtti::context& ctx,
                                mono::mono_object& obj,
                                const Invoker& mutable_field,
                                const var_info& info) -> inspect_result
    {
        inspect_result result;

        auto val = mutable_field.get_value(obj);

        auto& ec = ctx.get_cached<ecs>();
        auto& scene = ec.get_scene();
        auto e = scene.create_entity(val);

        auto attribs = mutable_field.get_attributes();
        auto tooltip_attrib = find_attribute("TooltipAttribute", attribs);

        std::string tooltip;
        if(tooltip_attrib.valid())
        {
            auto invoker = mono::make_field_invoker<std::string>(tooltip_attrib.get_type(), "tooltip");
            tooltip = invoker.get_value(tooltip_attrib);
        }

        rttr::variant var = e;

        {
            property_layout layout(mutable_field.get_name(), tooltip);
            result |= inspect_var(ctx, var, info);
        }

        if(result.changed)
        {
            auto v = var.get_value<entt::handle>();
            mutable_field.set_value(obj, v.entity());
        }

        return result;
    }

    static auto inspect_field(rtti::context& ctx, mono::mono_object& obj, mono::mono_field& field, const var_info& info)
        -> inspect_result
    {
        auto invoker = mono::make_field_invoker<entt::entity>(field);

        var_info field_info;
        field_info.is_property = true;
        field_info.read_only = info.read_only || field.is_readonly();

        return inspect_invoker(ctx, obj, invoker, field_info);
    }

    static auto inspect_property(rtti::context& ctx,
                                 mono::mono_object& obj,
                                 mono::mono_property& field,
                                 const var_info& info) -> inspect_result
    {
        auto invoker = mono::make_property_invoker<entt::entity>(field);

        var_info field_info;
        field_info.is_property = true;
        field_info.read_only = info.read_only || field.is_readonly();

        return inspect_invoker(ctx, obj, invoker, info);
    }
};

template<typename T>
struct mono_inspector<asset_handle<T>>
{
    template<typename Invoker>
    static auto inspect_invoker(rtti::context& ctx,
                                mono::mono_object& obj,
                                const Invoker& mutable_field,
                                const var_info& info) -> inspect_result
    {
        inspect_result result;
        const auto& field_type = mutable_field.get_type();

        auto val = mutable_field.get_value(obj);

        auto prop = field_type.get_property("uid");
        auto mutable_prop = mono::make_property_invoker<hpp::uuid>(prop);

        asset_handle<T> asset;
        if(val)
        {
            auto uid = mutable_prop.get_value(val);

            auto& am = ctx.get_cached<asset_manager>();
            asset = am.get_asset<T>(uid);
        }

        auto attribs = mutable_field.get_attributes();
        auto tooltip_attrib = find_attribute("TooltipAttribute", attribs);

        std::string tooltip;
        if(tooltip_attrib.valid())
        {
            auto invoker = mono::make_field_invoker<std::string>(tooltip_attrib.get_type(), "tooltip");
            tooltip = invoker.get_value(tooltip_attrib);
        }

        rttr::variant var = asset;

        {
            property_layout layout(mutable_field.get_name(), tooltip);
            result |= inspect_var(ctx, var, info);
        }

        if(result.changed)
        {
            auto v = var.get_value<asset_handle<T>>();
            if(v && !val)
            {
                val = field_type.new_instance();
                mutable_field.set_value(obj, val);
            }

            if(val)
            {
                mutable_prop.set_value(val, v.uid());
            }
        }

        return result;
    }

    static auto inspect_field(rtti::context& ctx, mono::mono_object& obj, mono::mono_field& field, const var_info& info)
        -> inspect_result
    {
        auto invoker = mono::make_field_invoker<mono::mono_object>(field);

        var_info field_info;
        field_info.is_property = true;
        field_info.read_only = info.read_only || field.is_readonly();

        return inspect_invoker(ctx, obj, invoker, field_info);
    }

    static auto inspect_property(rtti::context& ctx,
                                 mono::mono_object& obj,
                                 mono::mono_property& field,
                                 const var_info& info) -> inspect_result
    {
        auto invoker = mono::make_property_invoker<mono::mono_object>(field);

        var_info field_info;
        field_info.is_property = true;
        field_info.read_only = info.read_only || field.is_readonly();

        return inspect_invoker(ctx, obj, invoker, info);
    }
};

auto inspector_mono_object::inspect(rtti::context& ctx,
                                    rttr::variant& var,
                                    const var_info& info,
                                    const meta_getter& get_metadata) -> inspect_result
{
    auto& data = var.get_value<mono::mono_object>();

    inspect_result result{};

    const auto& type = data.get_type();

    using mono_field_inspector =
        std::function<inspect_result(rtti::context&, mono::mono_object&, mono::mono_field&, const var_info&)>;

    auto get_field_inspector = [](const std::string& type_name) -> const mono_field_inspector&
    {
        // clang-format off
        static std::map<std::string, mono_field_inspector> reg = {
            {"SByte",   &mono_inspector<int8_t>::inspect_field},
            {"Byte",    &mono_inspector<uint8_t>::inspect_field},
            {"Int16",   &mono_inspector<int16_t>::inspect_field},
            {"UInt16",  &mono_inspector<uint16_t>::inspect_field},
            {"Int32",   &mono_inspector<int32_t>::inspect_field},
            {"UInt32",  &mono_inspector<uint32_t>::inspect_field},
            {"Int64",   &mono_inspector<int64_t>::inspect_field},
            {"UInt64",  &mono_inspector<uint64_t>::inspect_field},
            {"Boolean", &mono_inspector<bool>::inspect_field},
            {"Single",  &mono_inspector<float>::inspect_field},
            {"Double",  &mono_inspector<double>::inspect_field},
            {"Char",    &mono_inspector<char16_t>::inspect_field},
            {"String",  &mono_inspector<std::string>::inspect_field},
            {"Entity",  &mono_inspector<entt::handle>::inspect_field},
            {"Texture",         &mono_inspector<asset_handle<gfx::texture>>::inspect_field},
            {"Material",        &mono_inspector<asset_handle<material>>::inspect_field},
            {"Mesh",            &mono_inspector<asset_handle<mesh>>::inspect_field},
            {"AnimationClip",   &mono_inspector<asset_handle<animation_clip>>::inspect_field},
            {"Prefab",          &mono_inspector<asset_handle<prefab>>::inspect_field},
            {"Scene",           &mono_inspector<asset_handle<scene_prefab>>::inspect_field},
            {"PhysicsMaterial", &mono_inspector<asset_handle<physics_material>>::inspect_field},
            {"AudioClip",       &mono_inspector<asset_handle<audio_clip>>::inspect_field},
        };
        // clang-format on

        auto it = reg.find(type_name);
        if(it != reg.end())
        {
            return it->second;
        }
        static const mono_field_inspector empty;
        return empty;
    };

    auto fields = type.get_fields();
    for(auto& field : fields)
    {
        if(field.get_visibility() == mono::visibility::vis_public)
        {
            const auto& field_type = field.get_type();

            auto field_inspector = get_field_inspector(field_type.get_name());

            if(field_inspector)
            {
                result |= field_inspector(ctx, data, field, info);
            }
            else
            {
                var_info field_info;
                field_info.is_property = true;
                field_info.read_only = true;

                std::string unknown = field.get_type().get_name();
                rttr::variant var = unknown;
                {
                    property_layout layout(field.get_name());
                    result |= inspect_var(ctx, var, field_info);
                }
            }
        }
    }

    using mono_property_inspector =
        std::function<inspect_result(rtti::context&, mono::mono_object&, mono::mono_property&, const var_info&)>;

    auto get_property_inspector = [](const std::string& type_name) -> const mono_property_inspector&
    {
        // clang-format off
        static std::map<std::string, mono_property_inspector> reg = {
            {"SByte",   &mono_inspector<int8_t>::inspect_property},
            {"Byte",    &mono_inspector<uint8_t>::inspect_property},
            {"Int16",   &mono_inspector<int16_t>::inspect_property},
            {"UInt16",  &mono_inspector<uint16_t>::inspect_property},
            {"Int32",   &mono_inspector<int32_t>::inspect_property},
            {"UInt32",  &mono_inspector<uint32_t>::inspect_property},
            {"Int64",   &mono_inspector<int64_t>::inspect_property},
            {"UInt64",  &mono_inspector<uint64_t>::inspect_property},
            {"Boolean", &mono_inspector<bool>::inspect_property},
            {"Single",  &mono_inspector<float>::inspect_property},
            {"Double",  &mono_inspector<double>::inspect_property},
            {"Char",    &mono_inspector<char16_t>::inspect_property},
            {"String",  &mono_inspector<std::string>::inspect_property},
            {"Entity",  &mono_inspector<entt::handle>::inspect_property},
            {"Texture",         &mono_inspector<asset_handle<gfx::texture>>::inspect_property},
            {"Material",        &mono_inspector<asset_handle<material>>::inspect_property},
            {"Mesh",            &mono_inspector<asset_handle<mesh>>::inspect_property},
            {"AnimationClip",   &mono_inspector<asset_handle<animation_clip>>::inspect_property},
            {"Prefab",          &mono_inspector<asset_handle<prefab>>::inspect_property},
            {"Scene",           &mono_inspector<asset_handle<scene_prefab>>::inspect_property},
            {"PhysicsMaterial", &mono_inspector<asset_handle<physics_material>>::inspect_property},
            {"AudioClip",       &mono_inspector<asset_handle<audio_clip>>::inspect_property}
        };
        // clang-format on

        auto it = reg.find(type_name);
        if(it != reg.end())
        {
            return it->second;
        }
        static const mono_property_inspector empty;
        return empty;
    };

    auto properties = type.get_properties();
    for(auto& prop : properties)
    {
        if(prop.get_visibility() == mono::visibility::vis_public)
        {
            const auto& prop_type = prop.get_type();

            auto property_inspector = get_property_inspector(prop_type.get_name());

            if(property_inspector)
            {
                result |= property_inspector(ctx, data, prop, info);
            }
            else
            {
                var_info field_info;
                field_info.is_property = true;
                field_info.read_only = true;

                std::string unknown = prop.get_type().get_name();
                rttr::variant var = unknown;
                {
                    property_layout layout(prop.get_name());
                    result |= inspect_var(ctx, var, field_info);
                }
            }
        }
    }

    return result;
}

} // namespace ace
