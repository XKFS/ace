#include "game_panel.h"
#include "../panels_defs.h"

#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/rendering/ecs/components/camera_component.h>
#include <engine/rendering/ecs/systems/rendering_system.h>

namespace ace
{

void game_panel::init(rtti::context& ctx)
{
}

void game_panel::deinit(rtti::context& ctx)
{
}

void game_panel::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& path = ctx.get_cached<rendering_system>();
    auto& ec = ctx.get_cached<ecs>();
    auto& scene = ec.get_scene();

    path.prepare_scene(scene, dt);
}

void game_panel::on_frame_render(rtti::context& ctx, delta_t dt)
{
    if(!is_visible_)
    {
        return;
    }
    auto& path = ctx.get_cached<rendering_system>();
    auto& ec = ctx.get_cached<ecs>();
    auto& scene = ec.get_scene();

    path.render_scene(scene, dt);
}

void game_panel::on_frame_ui_render(rtti::context& ctx, const char* name)
{
    auto& ev = ctx.get_cached<events>();
    if(ImGui::Begin(name, nullptr, ImGuiWindowFlags_MenuBar))
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));
        ev.is_input_allowed = ImGui::IsWindowFocused();

        set_visible(true);
        draw_ui(ctx);

    }
    else
    {
        ev.is_input_allowed = false;
        set_visible(false);
    }
    ImGui::End();


}

void game_panel::draw_ui(rtti::context& ctx)
{
    draw_menubar(ctx);

    auto& ec = ctx.get_cached<ecs>();
    auto size = ImGui::GetContentRegionAvail();

    if(size.x > 0 && size.y > 0)
    {
        bool rendered = false;
        ec.get_scene().registry->view<camera_component>().each(
            [&](auto e, auto&& camera_comp)
            {
                camera_comp.set_viewport_size({static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y)});

                const auto& camera = camera_comp.get_camera();
                const auto& rview = camera_comp.get_render_view();
                const auto& obuffer = rview.fbo_safe_get("OBUFFER");

                if(obuffer)
                {
                    auto tex = obuffer->get_texture(0);
                    ImGui::Image(ImGui::ToId(tex), size);
                    rendered = true;
                }
            });

        if(!rendered)
        {
            static const auto text = "No cameras rendering";
            ImGui::SetCursorPosY(size.y * 0.5f);
            ImGui::AlignedItem(0.5f,
                               size.x,
                               ImGui::CalcTextSize(text).x,
                               []()
                               {
                                   ImGui::TextUnformatted(text);
                               });
        }
    }
}

void game_panel::draw_menubar(rtti::context& ctx)
{
    if(ImGui::BeginMenuBar())
    {
        ImGui::EndMenuBar();
    }
}

} // namespace ace
