#include "project_settings_panel.h"
#include "../panel.h"
#include "../panels_defs.h"

#include <editor/hub/panels/inspector_panel/inspectors/inspectors.h>
#include <editor/system/project_manager.h>
#include <engine/input/input.h>

#include <filedialog/filedialog.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ace
{

project_settings_panel::project_settings_panel(imgui_panels* parent) : parent_(parent)
{
}

void project_settings_panel::show(bool s)
{
    show_request_ = s;
}

void project_settings_panel::on_frame_ui_render(rtti::context& ctx, const char* name)
{
    if(show_request_)
    {
        ImGui::OpenPopup(name);
        show_request_ = false;
    }

    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size * 0.5f);
    bool show = true;
    if(ImGui::BeginPopupModal(name, &show))
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));

        draw_ui(ctx);

        ImGui::EndPopup();
    }
}

void project_settings_panel::draw_ui(rtti::context& ctx)
{
    // auto& pm = ctx.get_cached<project_manager>();
    // auto& settings = pm.get_settings();

    if(ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& input = ctx.get_cached<input_system>();

        ImGui::PushItemWidth(150.0f);
        for(const auto& kvp : input.mapper.keyboard_action_map_.entries_by_action_id_)
        {
            const auto& action = kvp.first;
            const auto& mappings = kvp.second;

            if(ImGui::TreeNode(action.c_str()))
            {
                for(const auto& mapping : mappings)
                {
                    if(&mapping != &mappings.front())
                        ImGui::Separator();

                    auto oskey = static_cast<os::key::code>(mapping.key);
                    auto key_name = os::key::to_string(oskey);

                    ImGui::LabelText("Key", "%s", key_name.c_str());
                    for(const auto& modifier : mapping.modifiers)
                    {
                        auto osmodifier = static_cast<os::key::code>(modifier);
                        auto modifier_name = os::key::to_string(osmodifier);

                        ImGui::LabelText("Modifier", "%s", modifier_name.c_str());
                    }

                    ImGui::LabelText("Analog Value", "%f", mapping.analog_value);
                }

                ImGui::TreePop();

            }
        }

        for(const auto& kvp : input.mapper.mouse_action_map_.entries_by_action_id_)
        {
            const auto& action = kvp.first;
            const auto& mappings = kvp.second;

            if(ImGui::TreeNode(action.c_str()))
            {
                for(const auto& mapping : mappings)
                {
                    if(&mapping != &mappings.front())
                        ImGui::Separator();

                    auto type = input::to_string(mapping.type);

                    ImGui::LabelText("Type", "%s", type.c_str());

                    auto range = input::to_string(mapping.range);

                    ImGui::LabelText("Range", "%s", range.c_str());

                    auto axis = static_cast<input::mouse_axis>(mapping.value);
                    auto mouse_axis = input::to_string(axis);

                    ImGui::LabelText("Axis", "%s", mouse_axis.c_str());
                }
                ImGui::TreePop();
            }


        }

        ImGui::PopItemWidth();
    }

    // if(inspect(ctx, settings).changed)
    // {
    //     pm.save_project_settings();
    // }
}

} // namespace ace
