#include "header_panel.h"
#include "../panel.h"
#include "../panels_defs.h"

#include <editor/editing/editing_manager.h>
#include <editor/system/project_manager.h>

#include <engine/assets/asset_manager.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/meta/ecs/entity.hpp>
#include <engine/rendering/renderer.h>
#include <engine/threading/threader.h>

#include <simulation/simulation.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ace
{

header_panel::header_panel(imgui_panels* parent) : parent_(parent)
{
}

void header_panel::draw_menubar_child(rtti::context& ctx)
{
    ImGuiWindowFlags headerFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar;
    const std::string childID = "HEADER_menubar";
    ImGui::BeginChild(childID.c_str(), ImVec2(0, ImGui::GetFrameHeight() - 2), false, headerFlags);

    // Draw menu bar.
    if(ImGui::BeginMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("New Scene", ImGui::GetKeyCombinationName(new_scene_key_).c_str()))
            {
                editor_actions::new_scene(ctx);
            }

            if(ImGui::MenuItem("Open Scene", ImGui::GetKeyCombinationName(open_scene_key_).c_str()))
            {
                editor_actions::open_scene(ctx);
            }

            if(ImGui::MenuItem("Save Scene...", ImGui::GetKeyCombinationName(save_scene_key_).c_str()))
            {
                editor_actions::save_scene(ctx);
            }

            if(ImGui::MenuItem("Save Scene As", ImGui::GetKeyCombinationName(save_scene_as_key_).c_str()))
            {
                editor_actions::save_scene_as(ctx);
            }

            if(ImGui::MenuItem("Close Project", nullptr))
            {
                editor_actions::close_project(ctx);
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Deploy"))
        {
            if(ImGui::MenuItem("Deploy Project"))
            {
                parent_->get_deploy_panel().show(true);
            }

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Help"))
        {
            if(ImGui::MenuItem("About"))
            {
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if(ImGui::IsCombinationKeyPressed(new_scene_key_))
    {
        editor_actions::new_scene(ctx);
    }
    else if(ImGui::IsCombinationKeyPressed(open_scene_key_))
    {
        editor_actions::open_scene(ctx);
    }
    else if(ImGui::IsCombinationKeyPressed(save_scene_as_key_))
    {
        editor_actions::save_scene_as(ctx);
    }
    else if(ImGui::IsCombinationKeyPressed(save_scene_key_))
    {
        editor_actions::save_scene(ctx);
    }

    ImGui::EndChild();
}

void header_panel::draw_play_toolbar(rtti::context& ctx, float headerSize)
{
    auto& ev = ctx.get_cached<events>();

    float width = ImGui::GetContentRegionAvail().x;

    auto window_pos = ImGui::GetWindowPos();
    auto window_size = ImGui::GetWindowSize();
    // Add a poly background for the logo.
    const ImVec2 logo_bounds = ImVec2(500, headerSize * 0.5f);
    const ImVec2 logo_pos = ImVec2(window_pos.x + window_size.x * 0.5f - logo_bounds.x * 0.5f, window_pos.y);

    ImVec2 points[5] = {ImVec2(logo_pos.x, logo_pos.y),
                        ImVec2(logo_pos.x + 20, logo_pos.y + logo_bounds.y + 4),
                        ImVec2(logo_pos.x + logo_bounds.x - 20, logo_pos.y + logo_bounds.y + 4),
                        ImVec2(logo_pos.x + logo_bounds.x, logo_pos.y),
                        ImVec2(logo_pos.x, logo_pos.y)};

    const ImU32 polyBackground = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
    auto polyBackgroundBorderColor = polyBackground;

    if(ev.is_playing)
    {
        polyBackgroundBorderColor = ImGui::GetColorU32(ImVec4(0.0f, 0.5f, 0.0f, 0.5f));
    }
    if(ev.is_paused)
    {
        polyBackgroundBorderColor = ImGui::GetColorU32(ImVec4(0.6f, 0.3f, 0.0f, 0.5f));
    }

    ImGui::GetWindowDrawList()->AddConvexPolyFilled(&points[0], 5, polyBackgroundBorderColor);
    // ImGui::GetWindowDrawList()->AddPolyline(&points[0], 4, polyBackgroundBorderColor, 0, 3);
    //  ImGui::GetWindowDrawList()->AddRectFilledMultiColor(logoPos,
    //                                                      logoPos + logoBounds,
    //                                                      polyBackground,
    //                                                      polyBackground,
    //                                                      polyBackgroundBorderColor,
    //                                                      polyBackgroundBorderColor);

    auto logo = fmt::format("Ace Editor <{}>", gfx::get_renderer_name(gfx::get_renderer_type()));
    auto logo_size = ImGui::CalcTextSize(logo.c_str());
    // Add animated logo.
    const ImVec2 logo_min =
        ImVec2(logo_pos.x + logo_bounds.x * 0.5f - logo_size.x * 0.5f, logo_pos.y + (logo_bounds.y - logo_size.y) * 0.5f);
    const ImVec2 logo_max = ImVec2(logo_min.x + logo_size.x, logo_min.y + logo_size.y);
    auto logo_border_color = ImGui::GetColorU32(ImGuiCol_Text);
    ImGui::GetWindowDrawList()->AddText(logo_min, logo_border_color, logo.c_str());

    const auto& style = ImGui::GetStyle();
    auto frame_padding = style.FramePadding;
    auto item_spacing = style.ItemSpacing;
    ImGui::AlignedItem(0.5f,
                       width,
                       ImGui::CalcTextSize(ICON_MDI_PLAY ICON_MDI_PAUSE ICON_MDI_SKIP_NEXT).x +
                           frame_padding.x * 6 + item_spacing.x * 2,
                       [&]()
                       {
                           ImGui::BeginGroup();
                           if(ImGui::Button(ev.is_playing ? ICON_MDI_STOP : ICON_MDI_PLAY))
                           {
                               ev.toggle_play_mode(ctx);

                               ImGui::FocusWindow(ImGui::FindWindowByName(ev.is_playing ? GAME_VIEW : SCENE_VIEW));
                           }
                           ImGui::SameLine();
                           if(ImGui::Button(ICON_MDI_PAUSE))
                           {
                               auto& ev = ctx.get_cached<events>();

                               bool was_playing = ev.is_playing;
                               ev.toggle_pause(ctx);
                           }
                           ImGui::SameLine();
                           ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
                           if(ImGui::Button(ICON_MDI_SKIP_NEXT))
                           {
                               auto& ev = ctx.get_cached<events>();
                               ev.skip_next_frame(ctx);
                           }
                           ImGui::PopItemFlag();
                           ImGui::SameLine();

                           auto& sim = ctx.get_cached<simulation>();

                           auto time_scale = sim.get_time_scale();
                           ImGui::SetNextItemWidth(100);
                           if(ImGui::SliderFloat("###Time Scale", &time_scale, 0.0f, 1.0f))
                           {
                               sim.set_time_scale(time_scale);
                           }
                           ImGui::SetItemTooltipCurrentViewport("%s", "Time scale.");
                           ImGui::SameLine();
                           auto& rend = ctx.get_cached<renderer>();
                           auto vsync = rend.get_vsync();
                           if(ImGui::Checkbox("Vsync", &vsync))
                           {
                               rend.set_vsync(vsync);
                           }

                           ImGui::EndGroup();
                       });
}

void header_panel::on_frame_ui_render(rtti::context& ctx, float header_size)
{
    ImGuiWindowFlags header_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration;
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, header_size));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    ImGui::SetNextWindowViewport(viewport->ID);

    bool open = ImGui::Begin("HEADER", nullptr, header_flags);

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    if(open)
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));

        // Draw a sep. child for the menu bar.
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_MenuBarBg));
        draw_menubar_child(ctx);
        ImGui::NewLine();
        draw_play_toolbar(ctx, header_size);
        ImGui::PopStyleColor();
    }

    ImGui::End();
}

} // namespace ace
