#include "hub.h"
#include <editor/events.h>
#include <editor/hub/panels/inspector_panel/inspectors/inspectors.h>
#include <editor/system/project_manager.h>
#include <engine/engine.h>
#include <engine/events.h>
#include <engine/rendering/renderer.h>
#include <hpp/optional.hpp>

#include <filedialog/filedialog.h>
#include <imgui/imgui.h>
#include <imgui_widgets/markdown.h>
#include <memory>

namespace ace
{

namespace
{
struct project_item
{
    ImGui::Font::Enum font{ImGui::Font::Count};
    float scale = 1.0f;
    std::string tag{};
    std::string name{};
};

void draw_item(const std::vector<project_item>& v, std::function<void(ImVec2)> callback)
{
    ImGui::BeginGroup();

    auto pos = ImGui::GetCursorPos();

    float height = 0;
    for(const auto& item : v)
    {
        if(item.font != ImGui::Font::Count)
        {
            ImGui::PushFont(item.font);
        }
        if(item.scale > 0)
        {
            ImGui::PushWindowFontScale(item.scale);
        }
        height += ImGui::GetFrameHeightWithSpacing();

        if(item.scale > 0)
        {
            ImGui::PopWindowFontScale();
        }

        if(item.font != ImGui::Font::Count)
        {
            ImGui::PopFont();
        }
    }
    ImVec2 item_size(ImGui::GetContentRegionAvail().x, height);

    callback(item_size);
    bool hovered = ImGui::IsItemHovered();
    ImGui::SetCursorPos(pos);
    ImGui::Dummy({});
    ImGui::Indent();

    // ImGui::BeginGroup();
    // {
    //     ImGui::AlignTextToFramePadding();
    //     ImGui::NewLine();

    //     ImGui::AlignTextToFramePadding();
    //     ImGui::TextUnformatted(fmt::format("{} {}.", 0, ICON_MDI_FOLDER).c_str());

    //     ImGui::AlignTextToFramePadding();
    //     ImGui::NewLine();
    // }
    // ImGui::EndGroup();
    // ImGui::SameLine(0.0f, 1.0f);
    ImGui::BeginGroup();

    size_t i = 0;
    for(const auto& item : v)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", item.tag.c_str());

        ImGui::SameLine();

        if(item.font != ImGui::Font::Count)
        {
            ImGui::PushFont(item.font);
        }
        if(item.scale > 0)
        {
            ImGui::PushWindowFontScale(item.scale);
        }

        ImGui::AlignTextToFramePadding();
        if(i == 0 && hovered)
        {
            ImGui::TextLink(fmt::format("{}", item.name).c_str());
        }
        else
        {
            ImGui::Text("%s", item.name.c_str());
        }

        if(item.scale > 0)
        {
            ImGui::PopWindowFontScale();
        }

        if(item.font != ImGui::Font::Count)
        {
            ImGui::PopFont();
        }

        i++;
    }
    ImGui::EndGroup();
    ImGui::Unindent();

    ImGui::EndGroup();
}
} // namespace

hub::hub(rtti::context& ctx)
{
    auto& ui_ev = ctx.get_cached<ui_events>();
    auto& ev = ctx.get_cached<events>();

    ev.on_frame_update.connect(sentinel_, this, &hub::on_frame_update);
    ev.on_frame_render.connect(sentinel_, this, &hub::on_frame_render);
    ev.on_play_begin.connect(sentinel_, -10000, this, &hub::on_play_begin);
    ev.on_script_recompile.connect(sentinel_, 10000, this, &hub::on_script_recompile);
    ev.on_os_event.connect(sentinel_, 10000, this, &hub::on_os_event);

    ui_ev.on_frame_ui_render.connect(sentinel_, this, &hub::on_frame_ui_render);
}

auto hub::init(rtti::context& ctx) -> bool
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    panels_.init(ctx);

    return true;
}

auto hub::deinit(rtti::context& ctx) -> bool
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    panels_.deinit(ctx);

    return true;
}

void hub::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& pm = ctx.get_cached<project_manager>();

    if(!pm.has_open_project())
    {
        return;
    }
    panels_.on_frame_update(ctx, dt);
}

void hub::on_frame_render(rtti::context& ctx, delta_t dt)
{
    auto& pm = ctx.get_cached<project_manager>();

    if(!pm.has_open_project())
    {
        return;
    }
    panels_.on_frame_render(ctx, dt);
}

void hub::on_frame_ui_render(rtti::context& ctx, delta_t dt)
{
    auto& pm = ctx.get_cached<project_manager>();

    if(!pm.has_open_project())
    {
        on_start_page_render(ctx);
    }
    else
    {
        on_opened_project_render(ctx);
    }
}

void hub::on_script_recompile(rtti::context& ctx, const std::string& protocol)
{
    panels_.get_console_log_panel().on_recompile();
}

void hub::on_play_begin(rtti::context& ctx)
{
    panels_.get_console_log_panel().on_play();
}

void hub::on_os_event(rtti::context& ctx, os::event& e)
{
    auto& pm = ctx.get_cached<project_manager>();
    if(!pm.has_open_project())
    {
        return;
    }

    if(e.type == os::events::window)
    {
        if(e.window.type == os::window_event_id::close)
        {
            auto window_id = e.window.window_id;

            auto& rend = ctx.get_cached<renderer>();
            auto& render_window = rend.get_main_window();
            if(render_window)
            {
                if(render_window->get_window().get_id() == window_id)
                {
                    auto result = native::message_box("Do you want to save changes you made?",
                                                      native::dialog_type::yes_no,
                                                      native::icon_type::question,
                                                      "Save before exit?");

                    switch(result)
                    {
                        case native::action_type::ok_or_yes:
                            editor_actions::save_scene(ctx);
                            break;
                        default:
                            break;
                    }

                }
            }
        }
    }
}

void hub::on_opened_project_render(rtti::context& ctx)
{
    panels_.on_frame_ui_render(ctx);
}

void hub::on_start_page_render(rtti::context& ctx)
{
    auto& pm = ctx.get_cached<project_manager>();

    auto on_create_project = [&](const std::string& p)
    {
        auto path = fs::path(p).make_preferred();
        pm.create_project(ctx, path);
    };
    auto on_open_project = [&](const std::string& p)
    {
        auto path = fs::path(p).make_preferred();
        pm.open_project(ctx, path);
    };

    //    ImGui::PushFont("standard_big");
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::Begin("START PAGE", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGui::OpenPopup("PROJECTS");
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size * 0.5f, ImGuiCond_Appearing);

    if(ImGui::BeginPopupModal("PROJECTS", nullptr, ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::BeginGroup();
        {
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar |
                                     ImGuiWindowFlags_NoSavedSettings;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

            if(ImGui::BeginChild("projects_content",
                                 ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, ImGui::GetContentRegionAvail().y),
                                 ImGuiChildFlags_AlwaysUseWindowPadding,
                                 flags))
            {
                const auto& rencent_projects = pm.get_options().recent_projects;
                for(const auto& prj : rencent_projects)
                {
                    auto p = fs::path(prj.path);
                    fs::error_code ec;
                    auto ftime = fs::last_write_time(p / "settings" / "deploy.cfg", ec);
                    auto system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

                    auto name = p.stem().string();
                    auto dir = p.parent_path().string();

                    project_item project_name{};
                    project_name.font = ImGui::Font::Black;
                    project_name.scale = 1.2;
                    project_name.tag = "Project";
                    project_name.name = name;

                    project_item location{};
                    location.font = ImGui::Font::Bold;
                    // location.size = 16;
                    location.tag = "Location";
                    location.name = dir;

                    project_item date{};
                    date.font = ImGui::Font::Medium;
                    // date.size = 16;
                    date.tag = "Last Modified";
                    date.name = fmt::format("{:%Y-%m-%d %H:%M:%S}", system_time);

                    draw_item({project_name, location, date},
                              [&](ImVec2 item_size)
                              {
                                  if(ImGui::Selectable(fmt::format("##{}", p.string()).c_str(), false, 0, item_size))
                                  {
                                      on_open_project(prj.path);
                                  }
                              });

                    if(&prj != &rencent_projects.back())
                    {
                        ImGui::PushStyleColor(ImGuiCol_Separator, ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_SeparatorHovered,
                                              ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_SeparatorActive,
                                              ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.5f));

                        ImGui::Separator();

                        ImGui::PopStyleColor(3);
                    }
                }
            }
            ImGui::EndChild();

            ImGui::PopStyleVar();
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        ImGui::BeginGroup();
        {
            ImGui::PushFont(ImGui::Font::Black);
            if(ImGui::Button("New Project", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() * 2)))
            {
                new_project_creator_ = true;
                std::string path;
                if(native::pick_folder_dialog(path))
                {
                    on_create_project(path);
                }
            }

            if(ImGui::Button("Open", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() * 2)))
            {
                std::string path;
                if(native::pick_folder_dialog(path))
                {
                    on_open_project(path);
                }
            }

            ImGui::PopFont();
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    ImGui::End();
}

} // namespace ace
