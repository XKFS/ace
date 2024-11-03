#include "inspector_script.h"
#include "imgui/imgui.h"
#include "inspectors.h"

namespace ace
{

inspect_result inspector_mono_object::inspect(rtti::context& ctx,
                                               rttr::variant& var,
                                               const var_info& info,
                                               const meta_getter& get_metadata)
{
    auto& data = var.get_value<mono::mono_object*>();

    inspect_result result{};

    const auto& type = data->get_type();

    auto fields = type.get_fields();
    for(const auto& field : fields)
    {
        if(field.get_visibility() == mono::visibility::vis_public)
        {
            ImGui::LabelText(field.get_type().get_name().c_str(), "%s", field.get_name().c_str());
        }
    }


    return result;
}

} // namespace ace
