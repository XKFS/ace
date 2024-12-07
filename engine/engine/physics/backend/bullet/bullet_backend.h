#pragma once
#include <engine/engine_export.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/physics/ecs/components/physics_component.h>
#include <engine/rendering/camera.h>
#include <graphics/debugdraw.h>

namespace ace
{
class camera;

struct bullet_backend
{
    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_play_begin(rtti::context& ctx);
    void on_play_end(rtti::context& ctx);
    void on_pause(rtti::context& ctx);
    void on_resume(rtti::context& ctx);
    void on_skip_next_frame(rtti::context& ctx);

    static void apply_explosion_force(physics_component& comp,
                                      float explosion_force,
                                      const math::vec3& explosion_position,
                                      float explosion_radius,
                                      float upwards_modifier,
                                      force_mode mode);
    static void apply_force(physics_component& comp, const math::vec3& force, force_mode mode);
    static void apply_torque(physics_component& comp, const math::vec3& toruqe, force_mode mode);
    static void clear_kinematic_velocities(physics_component& comp);

    static auto ray_cast(const math::vec3& origin,
                         const math::vec3& direction,
                         float max_distance,
                         int layer_mask,
                         bool query_sensors) -> hpp::optional<raycast_hit>;

    static auto ray_cast_all(const math::vec3& origin,
                         const math::vec3& direction,
                         float max_distance,
                         int layer_mask,
                         bool query_sensors) -> std::vector<raycast_hit>;

    static void on_create_component(entt::registry& r, entt::entity e);
    static void on_destroy_component(entt::registry& r, entt::entity e);
    static void on_destroy_bullet_rigidbody_component(entt::registry& r, entt::entity e);

    static void draw_system_gizmos(rtti::context& ctx, const camera& cam, gfx::dd_raii& dd);
    static void draw_gizmo(rtti::context& ctx, physics_component& comp, const camera& cam, gfx::dd_raii& dd);
};
} // namespace ace
