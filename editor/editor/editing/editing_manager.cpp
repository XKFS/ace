#include "editing_manager.h"
#include <engine/ecs/components/id_component.h>
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/meta/ecs/entity.hpp>
#include <engine/scripting/ecs/systems/script_system.h>
#include <imgui_widgets/gizmo.h>

namespace ace
{

auto editing_manager::init(rtti::context& ctx) -> bool
{
    auto& ev = ctx.get_cached<events>();

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
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    save_checkpoint(ctx);
    auto& scripting = ctx.get_cached<script_system>();
    scripting.unload_app_domain();
    scripting.wait_for_jobs_to_finish(ctx);
    scripting.load_app_domain(ctx, false);
    load_checkpoint(ctx, true);
}

void editing_manager::on_play_after_end(rtti::context& ctx)
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    auto& scripting = ctx.get_cached<script_system>();
    scripting.unload_app_domain();
    scripting.load_app_domain(ctx, false);
    load_checkpoint(ctx, false);
}

void editing_manager::on_script_recompile(rtti::context& ctx, const std::string& protocol)
{
    save_checkpoint(ctx);
    auto& scripting = ctx.get_cached<script_system>();
    scripting.unload_app_domain();
    scripting.load_app_domain(ctx, false);
    load_checkpoint(ctx, true);
}

void editing_manager::save_checkpoint(rtti::context& ctx)
{
    auto& ec = ctx.get_cached<ecs>();
    auto& scn = ec.get_scene();

    if(is_selected_type<entt::handle>())
    {
        auto sel = selection_data.object.get_value<entt::handle>();
        if(sel)
        {
            auto& id_comp = sel.get_or_emplace<id_component>();
            if(id_comp.id.is_nil())
            {
                id_comp.id = generate_uuid();
            }
        }
        
        unselect();
    }

    scene_cache_ = {};
    scene_cache_source_ = scn.source;
    // first save scene
    save_to_stream(scene_cache_, scn);
}

void editing_manager::load_checkpoint(rtti::context& ctx, bool recover_selection)
{
    auto& ec = ctx.get_cached<ecs>();
    auto& scn = ec.get_scene();

    // clear scene
    scn.unload();

    // recover scene with the new domain
    load_from_stream(scene_cache_, scn);

    scn.source = scene_cache_source_;

    entt::handle entity;
    scn.registry->view<id_component>().each(
        [&](auto e, auto&& comp)
        {
            entity = scn.create_entity(e);
        });

    if(entity)
    {
        entity.remove<id_component>();

        if(recover_selection)
        {
            select(entity);
        }
    }
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
