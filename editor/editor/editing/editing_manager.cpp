#include "editing_manager.h"
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/meta/ecs/entity.hpp>
#include <engine/scripting/ecs/systems/script_system.h>
#include <imgui_widgets/gizmo.h>

namespace ace
{

auto editing_manager::init(rtti::context& ctx) -> bool
{
    auto& ev = ctx.get<events>();

    ev.on_play_before_begin.connect(sentinel_, 1000, this, &editing_manager::on_play_before_begin);
    ev.on_play_after_end.connect(sentinel_, -1000, this, &editing_manager::on_play_after_end);
    ev.on_frame_update.connect(sentinel_, 1000, this, &editing_manager::on_frame_update);
    ev.on_script_recompile.connect(sentinel_, 1000, this, &editing_manager::on_script_recompile);

    return true;
}

auto editing_manager::deinit(rtti::context& ctx) -> bool
{
    scene_cache_ = {};
    scene_cache_source_ = {};

    unselect();
    unfocus();
    return true;
}

void editing_manager::on_play_before_begin(rtti::context& ctx)
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    unselect();
    unfocus();

    save_checkpoint(ctx);
    auto& scripting = ctx.get<script_system>();
    scripting.unload_app_domain();
    scripting.load_app_domain(ctx, false);
    load_checkpoint(ctx);
}

void editing_manager::on_play_after_end(rtti::context& ctx)
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    unselect();
    unfocus();

    auto& scripting = ctx.get<script_system>();
    scripting.unload_app_domain();
    scripting.load_app_domain(ctx, false);
    load_checkpoint(ctx);
}

void editing_manager::on_script_recompile(rtti::context& ctx, const std::string& protocol)
{
    if(protocol != "app")
    {
        return;
    }

    save_checkpoint(ctx);
    auto& scripting = ctx.get<script_system>();
    scripting.unload_app_domain();
    scripting.load_app_domain(ctx, false);
    load_checkpoint(ctx);
}

void editing_manager::save_checkpoint(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    scene_cache_ = {};
    scene_cache_source_ = scn.source;
    // first save scene
    save_to_stream(scene_cache_, scn);
}

void editing_manager::load_checkpoint(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    // clear scene
    scn.unload();

    // recover scene with the new domain
    load_from_stream(scene_cache_, scn);

    scn.source = scene_cache_source_;
}

void editing_manager::on_frame_update(rtti::context& ctx, delta_t)
{
    if(focused_data.frames > 0)
    {
        focused_data.frames--;
    }

    if(focused_data.frames == 0)
    {
        unfocus();
    }
}

void editing_manager::select(rttr::variant object)
{
    selection_data.object = object;
}

void editing_manager::focus(rttr::variant object)
{
    focused_data.object = object;
    focused_data.frames = 100;
}

void editing_manager::focus_path(const fs::path& object)
{
    focused_data.focus_path = object;
}

void editing_manager::unselect()
{
    selection_data = {};
    ImGuizmo::Enable(false);
    ImGuizmo::Enable(true);
}

void editing_manager::unfocus()
{
    focused_data = {};
}

void editing_manager::close_project()
{
    unselect();
    unfocus();
}
} // namespace ace
