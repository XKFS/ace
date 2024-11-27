#include "inspector_script.h"
#include "inspectors.h"
#include <engine/assets/asset_manager.h>
#include <monopp/mono_field_invoker.h>
#include <monopp/mono_property_invoker.h>

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
auto inspect_mono_field(rtti::context& ctx, mono::mono_object& obj, mono::mono_field& field, const var_info& info)
    -> inspect_result
{
    inspect_result result;

    var_info field_info;
    field_info.is_property = true;
    field_info.read_only = info.read_only || field.is_readonly();

    auto mutable_field = mono::make_field_invoker<T>(field);
    auto val = mutable_field.get_value(obj);

    auto attribs = field.get_attributes();
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
        property_layout layout(field.get_name(), tooltip);
        result |= inspect_var(ctx, var, field_info, getter);
    }

    if(result.changed)
    {
        val = var.get_value<T>();
        mutable_field.set_value(obj, val);
    }

    return result;
}

template<>
auto inspect_mono_field<entt::handle>(rtti::context& ctx, mono::mono_object& obj, mono::mono_field& field, const var_info& info)
    -> inspect_result
{
    inspect_result result;

    var_info field_info;
    field_info.is_property = true;
    field_info.read_only = info.read_only || field.is_readonly();

    auto mutable_field = mono::make_field_invoker<entt::entity>(field);
    auto val = mutable_field.get_value(obj);

    auto& ec = ctx.get_cached<ecs>();
    auto& scene = ec.get_scene();
    auto e = scene.create_entity(val);

    auto attribs = field.get_attributes();
    auto tooltip_attrib = find_attribute("TooltipAttribute", attribs);

    std::string tooltip;
    if(tooltip_attrib.valid())
    {
        auto invoker = mono::make_field_invoker<std::string>(tooltip_attrib.get_type(), "tooltip");
        tooltip = invoker.get_value(tooltip_attrib);
    }


    rttr::variant var = e;

    {
        property_layout layout(field.get_name(), tooltip);
        result |= inspect_var(ctx, var, field_info);
    }

    if(result.changed)
    {
        auto v = var.get_value<entt::handle>();
        mutable_field.set_value(obj, v.entity());
    }

    return result;
}

// template<>
// auto inspect_mono_field<asset_handle<prefab>>(rtti::context& ctx, mono::mono_object& obj, mono::mono_field& field, const var_info& info)
//     -> inspect_result
// {
//     inspect_result result;

//     var_info field_info;
//     field_info.is_property = true;
//     field_info.read_only = info.read_only || field.is_readonly();

//     auto mutable_field = mono::make_field_invoker<mono::mono_object>(field);
//     auto val = mutable_field.get_value(obj);

//     auto prop = field.get_type().get_property("uid");
//     auto mutable_prop = mono::make_property_invoker<hpp::uuid>(prop);
//     auto uid = mutable_prop.get_value(val);

//     auto& am = ctx.get_cached<asset_manager>();
//     auto handle = am.get_asset<prefab>(uid);

//     auto attribs = field.get_attributes();
//     auto tooltip_attrib = find_attribute("TooltipAttribute", attribs);

//     std::string tooltip;
//     if(tooltip_attrib.valid())
//     {
//         auto invoker = mono::make_field_invoker<std::string>(tooltip_attrib.get_type(), "tooltip");
//         tooltip = invoker.get_value(tooltip_attrib);
//     }


//     rttr::variant var = handle;

//     {
//         property_layout layout(field.get_name(), tooltip);
//         result |= inspect_var(ctx, var, field_info);
//     }

//     if(result.changed)
//     {
//         auto v = var.get_value<asset_handle<prefab>>();
//         mutable_prop.set_value(val, v.uid());
//     }

//     return result;
// }

template<typename T>
auto inspect_mono_property(rtti::context& ctx, mono::mono_object& obj, mono::mono_property& prop, const var_info& info)
    -> inspect_result
{
    inspect_result result;

    var_info prop_info;
    prop_info.is_property = true;
    prop_info.read_only = info.read_only;

    auto mutable_prop = mono::make_property_invoker<T>(prop);
    auto val = mutable_prop.get_value(obj);

    rttr::variant var = val;

    ImGui::PushReadonly(prop_info.read_only);

    {
        property_layout layout(prop.get_name());
        result |= inspect_var(ctx, var, prop_info);
    }

    if(result.changed)
    {
        val = var.get_value<T>();
        mutable_prop.set_value(obj, val);
    }

    ImGui::PopReadonly();

    return result;
}

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
        static std::map<std::string, mono_field_inspector> reg = {{"SByte", &inspect_mono_field<int8_t>},
                                                                  {"Byte", &inspect_mono_field<uint8_t>},
                                                                  {"Int16", &inspect_mono_field<int16_t>},
                                                                  {"UInt16", &inspect_mono_field<uint16_t>},
                                                                  {"Int32", &inspect_mono_field<int32_t>},
                                                                  {"UInt32", &inspect_mono_field<uint32_t>},
                                                                  {"Int64", &inspect_mono_field<int64_t>},
                                                                  {"UInt64", &inspect_mono_field<uint64_t>},
                                                                  {"Boolean", &inspect_mono_field<bool>},
                                                                  {"Single", &inspect_mono_field<float>},
                                                                  {"Double", &inspect_mono_field<double>},
                                                                  {"Char", &inspect_mono_field<char16_t>},
                                                                  {"String", &inspect_mono_field<std::string>},
                                                                  {"Entity", &inspect_mono_field<entt::handle>}};


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
        static std::map<std::string, mono_property_inspector> reg = {{"SByte", &inspect_mono_property<int8_t>},
                                                                     {"Byte", &inspect_mono_property<uint8_t>},
                                                                     {"Int16", &inspect_mono_property<int16_t>},
                                                                     {"UInt16", &inspect_mono_property<uint16_t>},
                                                                     {"Int32", &inspect_mono_property<int32_t>},
                                                                     {"UInt32", &inspect_mono_property<uint32_t>},
                                                                     {"Int64", &inspect_mono_property<int64_t>},
                                                                     {"UInt64", &inspect_mono_property<uint64_t>},
                                                                     {"Boolean", &inspect_mono_property<bool>},
                                                                     {"Single", &inspect_mono_property<float>},
                                                                     {"Double", &inspect_mono_property<double>},
                                                                     {"Char", &inspect_mono_property<char16_t>},
                                                                     {"String", &inspect_mono_property<std::string>},
                                                                     {"Entity", &inspect_mono_property<entt::handle>}};

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
