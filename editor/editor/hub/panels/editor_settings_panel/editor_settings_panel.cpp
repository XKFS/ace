#include "editor_settings_panel.h"
#include "../panel.h"

#include <editor/hub/panels/inspector_panel/inspectors/inspectors.h>
#include <editor/system/project_manager.h>
#include <engine/input/input.h>

#include <filedialog/filedialog.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ace
{

editor_settings_panel::editor_settings_panel(imgui_panels* parent) : parent_(parent)
{
}

void editor_settings_panel::show(bool s)
{
    show_request_ = s;
}

void editor_settings_panel::on_frame_ui_render(rtti::context& ctx, const char* name)
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

void editor_settings_panel::draw_ui(rtti::context& ctx)
{
    auto& pm = ctx.get_cached<project_manager>();
    auto& settings = pm.get_editor_settings();

    inspect_result result;
    if(ImGui::CollapsingHeader("External Tools", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TreePush("Debugger");
        result |= inspect(ctx, settings.external_tools);
        ImGui::TreePop();
    }

    if(ImGui::CollapsingHeader("Debugger", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TreePush("Debugger");
        result |= inspect(ctx, settings.debugger);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", "Requires an editor restart to apply these changes.");
        ImGui::TreePop();
    }

    if(result.edit_finished)
    {
        pm.save_editor_settings();
    }
}

} // namespace ace
