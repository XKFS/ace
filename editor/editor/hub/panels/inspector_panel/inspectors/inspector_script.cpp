#include "inspector_script.h"
#include "inspectors.h"

#include <monopp/mono_field_invoker.h>
#include <monopp/mono_property_invoker.h>

namespace ace
{

template<typename T>
inspect_result inspect_mono_field(rtti::context& ctx,
                                  mono::mono_object& obj,
                                  mono::mono_field& field,
                                  const var_info& info)
{
    inspect_result result;

    var_info field_info;
    field_info.is_property = true;
    field_info.read_only = info.read_only || field.is_readonly();

    auto mutable_field = mono::make_field_invoker<T>(field);
    auto val = mutable_field.get_value(obj);

    rttr::variant var = val;

    {
        property_layout layout(field.get_name());
        result |= inspect_var(ctx, var, field_info);
    }

    if(result.changed)
    {
        val = var.get_value<T>();
        mutable_field.set_value(obj, val);
    }

    return result;
}

template<typename T>
inspect_result inspect_mono_property(rtti::context& ctx,
                                     mono::mono_object& obj,
                                     mono::mono_property& prop,
                                     const var_info& info)
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

inspect_result inspector_mono_object::inspect(rtti::context& ctx,
                                              rttr::variant& var,
                                              const var_info& info,
                                              const meta_getter& get_metadata)
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
                                                                  {"String", &inspect_mono_field<std::string>}};

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
                                                                     {"String", &inspect_mono_property<std::string>}};

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
        }
    }

    return result;
}

} // namespace ace
