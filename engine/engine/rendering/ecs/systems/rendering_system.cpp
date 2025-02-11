#include "rendering_system.h"

#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>

#include <engine/animation/ecs/systems/animation_system.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/systems/transform_system.h>
#include <engine/engine.h>
#include <engine/rendering/ecs/components/camera_component.h>
#include <engine/rendering/ecs/components/model_component.h>
#include <engine/rendering/ecs/systems/model_system.h>
#include <engine/rendering/ecs/systems/camera_system.h>
#include <engine/rendering/ecs/systems/reflection_probe_system.h>

namespace ace
{

auto rendering_system::init(rtti::context& ctx) -> bool
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto rendering_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void rendering_system::prepare_scene(scene& scn, delta_t dt)
{
    auto& ctx = engine::context();
    ctx.get_cached<transform_system>().on_frame_update(scn, dt);
    ctx.get_cached<camera_system>().on_frame_update(scn, dt);
    ctx.get_cached<model_system>().on_frame_update(scn, dt);
    ctx.get_cached<animation_system>().on_frame_update(scn, dt);
    ctx.get_cached<reflection_probe_system>().on_frame_update(scn, dt);
}

auto rendering_system::render_scene(camera_component& camera_comp, scene& scn, delta_t dt) -> gfx::frame_buffer::ptr
{
    auto& pipeline_data = camera_comp.get_pipeline_data();
    auto& camera = pipeline_data.get_camera();
    auto& pipeline = pipeline_data.get_pipeline();
    auto& rview = camera_comp.get_render_view();

    return pipeline->run_pipeline(scn, camera, rview, dt);
}

auto rendering_system::render_scene(scene& scn, delta_t dt) -> gfx::frame_buffer::ptr
{
    gfx::frame_buffer::ptr output{};
    scn.registry->view<camera_component>().each(
        [&](auto e, auto&& camera_comp)
        {
            output = render_scene(camera_comp, scn, dt);
            return;
        });

    return output;
}

void rendering_system::render_scene(const gfx::frame_buffer::ptr& output,
                                  camera_component& camera_comp,
                                  scene& scn,
                                  delta_t dt)
{
    auto& pipeline_data = camera_comp.get_pipeline_data();
    auto& camera = pipeline_data.get_camera();
    auto& pipeline = pipeline_data.get_pipeline();
    auto& rview = camera_comp.get_render_view();

    pipeline->run_pipeline(output, scn, camera, rview, dt);
}

void rendering_system::render_scene(const gfx::frame_buffer::ptr& output, scene& scn, delta_t dt)
{
    scn.registry->view<camera_component>().each(
        [&](auto e, auto&& camera_comp)
        {
            render_scene(output, camera_comp, scn, dt);
        });
}

} // namespace ace
