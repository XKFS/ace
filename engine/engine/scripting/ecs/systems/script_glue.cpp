#include "script_system.h"
#include <engine/ecs/ecs.h>
#include <engine/events.h>

#include <engine/engine.h>
#include <monopp/mono_exception.h>
#include <monopp/mono_internal_call.h>
#include <monopp/mono_jit.h>
#include <monort/mono_pod_wrapper.h>
#include <monort/monort.h>

#include <engine/scripting/ecs/components/script_component.h>

#include <engine/assets/asset_manager.h>
#include <engine/meta/ecs/components/all_components.h>

#include <filesystem/filesystem.h>
#include <logging/logging.h>

namespace mono
{
namespace managed_interface
{

struct vector2
{
    float x;
    float y;
};
template<>
inline auto converter::convert(const math::vec2& v) -> vector2
{
    return {v.x, v.y};
}

template<>
inline auto converter::convert(const vector2& v) -> math::vec2
{
    return {v.x, v.y};
}

struct vector3
{
    float x;
    float y;
    float z;
};
template<>
inline auto converter::convert(const math::vec3& v) -> vector3
{
    return {v.x, v.y, v.z};
}

template<>
inline auto converter::convert(const vector3& v) -> math::vec3
{
    return {v.x, v.y, v.z};
}

struct vector4
{
    float x;
    float y;
    float z;
    float w;
};
template<>
inline auto converter::convert(const math::vec4& v) -> vector4
{
    return {v.x, v.y, v.z, v.w};
}

template<>
inline auto converter::convert(const vector4& v) -> math::vec4
{
    return {v.x, v.y, v.z, v.w};
}

struct quaternion
{
    float x;
    float y;
    float z;
    float w;
};
template<>
inline auto converter::convert(const math::quat& q) -> quaternion
{
    return {q.x, q.y, q.z, q.w};
}

template<>
inline auto converter::convert(const quaternion& q) -> math::quat
{
    return math::quat::wxyz(q.w, q.x, q.y, q.z);
}
} // namespace managed_interface

register_basic_mono_converter_for_pod(math::vec2, managed_interface::vector2);
register_basic_mono_converter_for_pod(math::vec3, managed_interface::vector3);
register_basic_mono_converter_for_pod(math::vec4, managed_interface::vector4);
register_basic_mono_converter_for_pod(math::quat, managed_interface::quaternion);

} // namespace mono

namespace ace
{
namespace
{

auto get_entity_from_id(entt::entity id) -> entt::handle
{
    if(id == entt::entity(0))
    {
        return {};
    }
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    return ec.get_scene().create_entity(id);
}

void internal_m2n_load_scene(const std::string& key)
{
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();

    auto& am = ctx.get<asset_manager>();
    ec.get_scene().load_from(am.get_asset<scene_prefab>(key));
}

void internal_m2n_create_scene(const mono::mono_object& this_ptr)
{
    mono::ignore(this_ptr);
}

void internal_m2n_destroy_scene(const mono::mono_object& this_ptr)
{
    mono::ignore(this_ptr);
}

auto internal_m2n_create_entity(const std::string& tag) -> entt::entity
{
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    auto e = ec.get_scene().create_entity(tag);

    return e.entity();
}

auto internal_m2n_create_entity_from_prefab_uid(const hpp::uuid& uid) -> entt::entity
{
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    auto& am = ctx.get<asset_manager>();

    auto pfb = am.get_asset<prefab>(uid);
    auto e = ec.get_scene().instantiate(pfb);

    return e.entity();
}

auto internal_m2n_create_entity_from_prefab_key(const std::string& key) -> entt::entity
{
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    auto& am = ctx.get<asset_manager>();

    auto pfb = am.get_asset<prefab>(key);
    auto e = ec.get_scene().instantiate(pfb);

    return e.entity();
}

auto internal_m2n_clone_entity(entt::entity id) -> entt::entity
{
    auto e = get_entity_from_id(id);
    if(e)
    {
        auto& ctx = engine::context();
        auto& ec = ctx.get<ecs>();
        auto cloned = ec.get_scene().clone_entity(e);
        return cloned.entity();
    }

    entt::handle invalid;
    return invalid.entity();
}

auto internal_m2n_destroy_entity(entt::entity id) -> bool
{
    auto e = get_entity_from_id(id);
    if(e)
    {
        e.destroy();
        return true;
    }
    return false;
}

auto internal_m2n_is_entity_valid(entt::entity id) -> bool
{
    auto e = get_entity_from_id(id);
    return e.valid();
}

auto internal_m2n_find_entity_by_tag(const std::string& tag) -> uint32_t
{
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    auto view = registry.view<tag_component>();

    for(const auto& e : view)
    {
        if(registry.get<tag_component>(e).tag == tag)
        {
            return static_cast<uint32_t>(e);
        }
    }

    const entt::handle invalid;
    return static_cast<uint32_t>(invalid.entity());
}

struct native_comp_lut
{
    auto is_valid() const -> bool
    {
        return add_native != nullptr;
    }
    std::function<bool(const mono::mono_type& type, entt::handle e)> add_native;
    std::function<bool(const mono::mono_type& type, entt::handle e)> has_native;
    std::function<bool(const mono::mono_type& type, entt::handle e)> remove_native;

    static auto get_registry() -> std::unordered_map<std::string, native_comp_lut>&
    {
        static std::unordered_map<std::string, native_comp_lut> lut;
        return lut;
    }

    static auto get_action_table(const mono::mono_type& type) -> const native_comp_lut&
    {
        const auto& registry = get_registry();
        auto it = registry.find(type.get_name());
        if(it != registry.end())
        {
            return it->second;
        }

        static const native_comp_lut empty;
        return empty;
    }

    template<typename T>
    static auto register_native_component(const std::string& name)
    {
        native_comp_lut lut;
        lut.add_native = [name](const mono::mono_type& type, entt::handle e)
        {
            if(type.get_name() == name)
            {
                auto& native = e.get_or_emplace<T>();
                return true;
            }

            return false;
        };

        lut.has_native = [name](const mono::mono_type& type, entt::handle e)
        {
            if(type.get_name() == name)
            {
                return e.all_of<T>();
            }

            return false;
        };

        lut.remove_native = [name](const mono::mono_type& type, entt::handle e)
        {
            if(type.get_name() == name)
            {
                return e.remove<T>() > 0;
            }

            return false;
        };

        get_registry()[name] = lut;
    }
};

int register_componetns = []()
{
    native_comp_lut::register_native_component<id_component>("IdComponent");
    native_comp_lut::register_native_component<transform_component>("TransformComponent");
    native_comp_lut::register_native_component<model_component>("ModelComponent");
    native_comp_lut::register_native_component<camera_component>("CameraComponent");
    native_comp_lut::register_native_component<light_component>("LightComponent");
    native_comp_lut::register_native_component<reflection_probe_component>("ReflectionProbeComponent");
    native_comp_lut::register_native_component<physics_component>("PhysicsComponent");

    native_comp_lut::register_native_component<animation_component>("AnimationComponent");
    native_comp_lut::register_native_component<audio_listener_component>("AudioListenerComponent");
    native_comp_lut::register_native_component<audio_source_component>("AudioSourceComponent");

    return 0;
}();

auto internal_add_native_component(const mono::mono_type& type, entt::handle e, script_component& script_comp)
    -> mono::mono_object
{
    bool add = false;

    const auto& lut = native_comp_lut::get_action_table(type);
    if(lut.is_valid())
    {
        add = lut.add_native(type, e);
    }

    if(add)
    {
        auto comp = script_comp.get_native_component(type);

        if(!comp.scoped)
        {
            comp = script_comp.add_native_component(type);
        }
        return static_cast<mono::mono_object&>(comp.scoped->object);
    }

    return {};
}

auto internal_get_native_component(const mono::mono_type& type, entt::handle e, script_component& script_comp)
    -> mono::mono_object
{
    bool native = false;
    bool has = false;

    const auto& lut = native_comp_lut::get_action_table(type);
    if(lut.is_valid())
    {
        has = lut.has_native(type, e);
        native = true;
    }

    if(native)
    {
        auto comp = script_comp.get_native_component(type);
        if(has)
        {
            if(!comp.scoped)
            {
                comp = script_comp.add_native_component(type);
            }
            return static_cast<mono::mono_object&>(comp.scoped->object);
        }

        script_comp.remove_native_component(comp.scoped->object);
    }

    return {};
}

auto internal_remove_native_component(const mono::mono_object& obj, entt::handle e, script_component& script_comp)
    -> bool
{
    bool removed = false;
    const auto& lut = native_comp_lut::get_action_table(obj.get_type());
    if(lut.is_valid())
    {
        removed = lut.remove_native(obj.get_type(), e);
    }

    if(removed)
    {
        return script_comp.remove_native_component(obj);
    }

    return false;
}

auto internal_m2n_add_component(entt::entity id, const mono::mono_type& type) -> mono::mono_object
{
    auto e = get_entity_from_id(id);
    auto& script_comp = e.get_or_emplace<script_component>();

    if(auto native_comp = internal_add_native_component(type, e, script_comp))
    {
        return native_comp;
    }

    auto component = script_comp.add_script_component(type);
    return static_cast<mono::mono_object&>(component.scoped->object);
}

auto internal_m2n_get_component(entt::entity id, const mono::mono_type& type) -> mono::mono_object
{
    auto e = get_entity_from_id(id);
    auto& script_comp = e.get_or_emplace<script_component>();

    if(auto native_comp = internal_get_native_component(type, e, script_comp))
    {
        return native_comp;
    }

    auto component = script_comp.get_script_component(type);

    if(component.scoped)
    {
        return static_cast<mono::mono_object&>(component.scoped->object);
    }

    return {};
}

auto internal_m2n_has_component(entt::entity id, const mono::mono_type& type) -> bool
{
    auto comp = internal_m2n_get_component(id, type);

    return comp.valid();
}

auto internal_m2n_remove_component(entt::entity id, const mono::mono_object& comp) -> bool
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return false;
    }
    auto& script_comp = e.get_or_emplace<script_component>();

    if(internal_remove_native_component(comp, e, script_comp))
    {
        return true;
    }

    return script_comp.remove_script_component(comp);
}

//-------------------------------------------------------------------------
/*

  _      ____   _____
 | |    / __ \ / ____|
 | |   | |  | | |  __
 | |   | |  | | | |_ |
 | |___| |__| | |__| |
 |______\____/ \_____|


*/
//-------------------------------------------------------------------------

void internal_m2n_log_trace(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_TRACE_LOC(file.c_str(), line, func.c_str(), message);
}

void internal_m2n_log_info(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_INFO_LOC(file.c_str(), line, func.c_str(), message);
}

void internal_m2n_log_warning(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_WARNING_LOC(file.c_str(), line, func.c_str(), message);
}

void internal_m2n_log_error(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_ERROR_LOC(file.c_str(), line, func.c_str(), message);
}

//-------------------------------------------------------------------------
/*

  _______ _____            _   _  _____ ______ ____  _____  __  __
 |__   __|  __ \     /\   | \ | |/ ____|  ____/ __ \|  __ \|  \/  |
    | |  | |__) |   /  \  |  \| | (___ | |__ | |  | | |__) | \  / |
    | |  |  _  /   / /\ \ | . ` |\___ \|  __|| |  | |  _  /| |\/| |
    | |  | | \ \  / ____ \| |\  |____) | |   | |__| | | \ \| |  | |
    |_|  |_|  \_\/_/    \_\_| \_|_____/|_|    \____/|_|  \_\_|  |_|


*/
//-------------------------------------------------------------------------
auto internal_m2n_get_position_global(entt::entity id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_position_global();
}

void internal_m2n_set_position_global(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_position_global(value);
}

void internal_m2n_move_by_global(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.move_by_global(value);
}

auto internal_m2n_get_position_local(entt::entity id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_position_local();
}

void internal_m2n_set_position_local(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_position_local(value);
}

void internal_m2n_move_by_local(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.move_by_local(value);
}

//--------------------------------------------------
auto internal_m2n_get_rotation_euler_global(entt::entity id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_rotation_euler_global();
}

void internal_m2n_rotate_by_euler_global(entt::entity id, const math::vec3& amount)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.rotate_by_euler_global(amount);
}

void internal_m2n_rotate_axis_global(entt::entity id, float degrees, const math::vec3& axis)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.rotate_axis_global(degrees, axis);
}

void internal_m2n_look_at(entt::entity id, const math::vec3& point, const math::vec3& up)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.look_at(point, up);
}

void internal_m2n_set_rotation_euler_global(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_rotation_euler_global(value);
}

auto internal_m2n_get_rotation_euler_local(entt::entity id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_rotation_euler_local();
}

void internal_m2n_set_rotation_euler_local(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_rotation_euler_local(value);
}

void internal_m2n_rotate_by_euler_local(entt::entity id, const math::vec3& amount)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.rotate_by_euler_local(amount);
}

auto internal_m2n_get_rotation_global(entt::entity id) -> math::quat
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_rotation_global();
}

void internal_m2n_set_rotation_global(entt::entity id, const math::quat& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_rotation_global(value);
}

void internal_m2n_rotate_by_global(entt::entity id, const math::quat& amount)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.rotate_by_global(amount);
}

auto internal_m2n_get_rotation_local(entt::entity id) -> math::quat
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_rotation_local();
}

void internal_m2n_set_rotation_local(entt::entity id, const math::quat& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_rotation_local(value);
}

void internal_m2n_rotate_by_local(entt::entity id, const math::quat& amount)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.rotate_by_local(amount);
}

//--------------------------------------------------
auto internal_m2n_get_scale_global(entt::entity id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_scale_global();
}

void internal_m2n_set_scale_global(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_scale_global(value);
}

void internal_m2n_scale_by_global(entt::entity id, const math::vec3& amount)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.scale_by_global(amount);
}


auto internal_m2n_get_scale_local(entt::entity id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_scale_local();
}

void internal_m2n_set_scale_local(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_scale_local(value);
}

void internal_m2n_scale_by_local(entt::entity id, const math::vec3& amount)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.scale_by_local(amount);
}

//--------------------------------------------------
auto internal_m2n_get_skew_global(entt::entity id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_skew_global();
}

void internal_m2n_setl_skew_globa(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_skew_global(value);
}

auto internal_m2n_get_skew_local(entt::entity id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_skew_local();
}

void internal_m2n_set_skew_local(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_skew_local(value);
}

//------------------------------
void internal_m2n_apply_impulse(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto comp = e.try_get<physics_component>();

    if(!comp)
    {
        return;
    }

    comp->apply_impulse(value);
}

void internal_m2n_apply_torque_impulse(entt::entity id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }
    auto comp = e.try_get<physics_component>();

    if(!comp)
    {
        return;
    }

    comp->apply_torque_impulse(value);
}

//------------------------------
auto internal_m2n_from_euler_rad(const math::vec3& euler) -> math::quat
{
    return {euler};
}

auto internal_m2n_to_euler_rad(const math::quat& euler) -> math::vec3
{
    return math::eulerAngles(euler);
}

auto internal_m2n_angle_axis(float angle, const math::vec3& axis) -> math::quat
{
    return math::angleAxis(angle, axis);
}

auto internal_m2n_look_rotation(const math::vec3& a, const math::vec3& b) -> math::quat
{
    return {a, b};
}
auto internal_m2n_from_to_rotation(const math::vec3& from, const math::vec3& to) -> math::quat
{
    float angle = math::acos(math::clamp(math::dot(math::normalize(from), math::normalize(to)), -1.0f, 1.0f));
    math::vec3 axis = math::normalize(math::cross(from, to));
    return internal_m2n_angle_axis(angle, axis);
}

auto internal_m2n_get_asset_by_uuid(const hpp::uuid& uid, const mono::mono_type& type) -> hpp::uuid
{
    auto& ctx = engine::context();
    auto& am = ctx.get<asset_manager>();

    if(type.get_name() == "Prefab")
    {
        auto asset = am.get_asset<prefab>(uid);
        return asset.uid();
    }

    return {};
}

auto internal_m2n_get_asset_by_key(const std::string& key, const mono::mono_type& type) -> hpp::uuid
{
    auto& ctx = engine::context();
    auto& am = ctx.get<asset_manager>();

    if(type.get_name() == "Prefab")
    {
        auto asset = am.get_asset<prefab>(key);
        return asset.uid();
    }

    return {};
}

auto m2n_test_uuid(const hpp::uuid& uid) -> hpp::uuid
{
    APPLOG_INFO("{}:: From C# {}", __func__, hpp::to_string(uid));

    auto newuid = generate_uuid();
    APPLOG_INFO("{}:: New C++ {}", __func__, hpp::to_string(newuid));

    return newuid;
}

} // namespace

auto script_system::bind_internal_calls(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    {
        auto reg = mono::internal_call_registry("Ace.Core.Log");
        reg.add_internal_call("internal_m2n_log_trace", internal_call(internal_m2n_log_trace));
        reg.add_internal_call("internal_m2n_log_info", internal_call(internal_m2n_log_info));
        reg.add_internal_call("internal_m2n_log_warning", internal_call(internal_m2n_log_warning));
        reg.add_internal_call("internal_m2n_log_error", internal_call(internal_m2n_log_error));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.Scene");
        reg.add_internal_call("internal_m2n_load_scene", internal_call(internal_m2n_load_scene));
        reg.add_internal_call("internal_m2n_create_scene", internal_call(internal_m2n_create_scene));
        reg.add_internal_call("internal_m2n_destroy_scene", internal_call(internal_m2n_destroy_scene));
        reg.add_internal_call("internal_m2n_create_entity", internal_call(internal_m2n_create_entity));
        reg.add_internal_call("internal_m2n_create_entity_from_prefab_uid",
                              internal_call(internal_m2n_create_entity_from_prefab_uid));
        reg.add_internal_call("internal_m2n_create_entity_from_prefab_key",
                              internal_call(internal_m2n_create_entity_from_prefab_key));
        reg.add_internal_call("internal_m2n_destroy_entity", internal_call(internal_m2n_destroy_entity));
        reg.add_internal_call("internal_m2n_is_entity_valid", internal_call(internal_m2n_is_entity_valid));
        reg.add_internal_call("internal_m2n_find_entity_by_tag", internal_call(internal_m2n_find_entity_by_tag));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.Entity");
        reg.add_internal_call("internal_m2n_add_component", internal_call(internal_m2n_add_component));
        reg.add_internal_call("internal_m2n_get_component", internal_call(internal_m2n_get_component));
        reg.add_internal_call("internal_m2n_has_component", internal_call(internal_m2n_has_component));
        reg.add_internal_call("internal_m2n_remove_component", internal_call(internal_m2n_remove_component));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.TransformComponent");
        reg.add_internal_call("internal_m2n_get_position_global", internal_call(internal_m2n_get_position_global));
        reg.add_internal_call("internal_m2n_set_position_global", internal_call(internal_m2n_set_position_global));
        reg.add_internal_call("internal_m2n_move_by_global", internal_call(internal_m2n_move_by_global));

        reg.add_internal_call("internal_m2n_get_position_local", internal_call(internal_m2n_get_position_local));
        reg.add_internal_call("internal_m2n_set_position_local", internal_call(internal_m2n_set_position_local));
        reg.add_internal_call("internal_m2n_move_by_local", internal_call(internal_m2n_move_by_local));

        // Euler
        reg.add_internal_call("internal_m2n_get_rotation_euler_global",
                              internal_call(internal_m2n_get_rotation_euler_global));
        reg.add_internal_call("internal_m2n_set_rotation_euler_global",
                              internal_call(internal_m2n_set_rotation_euler_global));
        reg.add_internal_call("internal_m2n_rotate_by_euler_global",
                              internal_call(internal_m2n_rotate_by_euler_global));

        reg.add_internal_call("internal_m2n_get_rotation_euler_local",
                              internal_call(internal_m2n_get_rotation_euler_local));
        reg.add_internal_call("internal_m2n_set_rotation_euler_local",
                              internal_call(internal_m2n_set_rotation_euler_local));
        reg.add_internal_call("internal_m2n_rotate_by_euler_local", internal_call(internal_m2n_rotate_by_euler_local));

        // Quat
        reg.add_internal_call("internal_m2n_get_rotation_global", internal_call(internal_m2n_get_rotation_global));
        reg.add_internal_call("internal_m2n_set_rotation_global", internal_call(internal_m2n_set_rotation_global));
        reg.add_internal_call("internal_m2n_rotate_by_global", internal_call(internal_m2n_rotate_by_global));

        reg.add_internal_call("internal_m2n_get_rotation_local", internal_call(internal_m2n_get_rotation_local));
        reg.add_internal_call("internal_m2n_set_rotation_local", internal_call(internal_m2n_set_rotation_local));
        reg.add_internal_call("internal_m2n_rotate_by_local", internal_call(internal_m2n_rotate_by_local));

        // Other
        reg.add_internal_call("internal_m2n_rotate_axis_global", internal_call(internal_m2n_rotate_axis_global));
        reg.add_internal_call("internal_m2n_look_at", internal_call(internal_m2n_look_at));


        // Scale
        reg.add_internal_call("internal_m2n_get_scale_global", internal_call(internal_m2n_get_scale_global));
        reg.add_internal_call("internal_m2n_set_scale_global", internal_call(internal_m2n_set_scale_global));
        reg.add_internal_call("internal_m2n_scale_by_global", internal_call(internal_m2n_scale_by_local));

        reg.add_internal_call("internal_m2n_get_scale_local", internal_call(internal_m2n_get_scale_local));
        reg.add_internal_call("internal_m2n_set_scale_local", internal_call(internal_m2n_set_scale_local));
        reg.add_internal_call("internal_m2n_scale_by_local", internal_call(internal_m2n_scale_by_local));

        //Skew
        reg.add_internal_call("internal_m2n_get_skew_global", internal_call(internal_m2n_get_skew_global));
        reg.add_internal_call("internal_m2n_set_skew_globa", internal_call(internal_m2n_setl_skew_globa));
        reg.add_internal_call("internal_m2n_get_skew_local", internal_call(internal_m2n_get_skew_local));
        reg.add_internal_call("internal_m2n_set_skew_local", internal_call(internal_m2n_set_skew_local));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.PhysicsComponent");
        reg.add_internal_call("internal_m2n_apply_impulse", internal_call(internal_m2n_apply_impulse));
        reg.add_internal_call("internal_m2n_apply_torque_impulse", internal_call(internal_m2n_apply_torque_impulse));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.Assets");
        reg.add_internal_call("internal_m2n_get_asset_by_uuid", internal_call(internal_m2n_get_asset_by_uuid));
        reg.add_internal_call("internal_m2n_get_asset_by_key", internal_call(internal_m2n_get_asset_by_key));
    }

    {
        auto reg = mono::internal_call_registry("Quaternion");
        reg.add_internal_call("internal_m2n_from_euler_rad", internal_call(internal_m2n_from_euler_rad));
        reg.add_internal_call("internal_m2n_to_euler_rad", internal_call(internal_m2n_to_euler_rad));
        reg.add_internal_call("internal_m2n_from_to_rotation", internal_call(internal_m2n_from_to_rotation));
        reg.add_internal_call("internal_m2n_angle_axis", internal_call(internal_m2n_angle_axis));
        reg.add_internal_call("internal_m2n_look_rotation", internal_call(internal_m2n_look_rotation));
    }


    {
        auto reg = mono::internal_call_registry("Ace.Core.Tests");
        reg.add_internal_call("m2n_test_uuid", internal_call(m2n_test_uuid));
    }
    // mono::managed_interface::init(assembly);

    return true;
}

} // namespace ace
