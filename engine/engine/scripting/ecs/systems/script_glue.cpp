#include "script_system.h"
#include <engine/ecs/ecs.h>
#include <engine/events.h>

#include <engine/engine.h>
#include <monopp/mono_exception.h>
#include <monopp/mono_internal_call.h>
#include <monopp/mono_jit.h>
#include <monort/mono_pod_wrapper.h>
#include <monort/monort.h>

#include <engine/assets/asset_manager.h>
#include <engine/audio/ecs/components/audio_source_component.h>
#include <engine/input/input.h>
#include <engine/meta/ecs/components/all_components.h>
#include <engine/physics/ecs/systems/physics_system.h>
#include <engine/scripting/ecs/components/script_component.h>

#include <filesystem/filesystem.h>
#include <logging/logging.h>
#include <tweeny/tweeny.h>

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

struct raycast_hit
{
    entt::entity entity{};
    vector3 point{};
    vector3 normal{};
    float distance{};
};

struct ray
{
    vector3 origin{};
    vector3 direction{};
};

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
    auto& ec = ctx.get_cached<ecs>();

    return ec.get_scene().create_entity(id);
}

void raise_invalid_entity_exception()
{
    mono::raise_exception("System", "Exception", "Entity is invalid.");
}

template<typename T>
void raise_missing_component_exception()
{
    mono::raise_exception("System",
                          "Exception",
                          fmt::format("Entity does not have component of type {}.", hpp::type_name_str<T>()));
}

template<typename T>
auto safe_get_component(entt::entity id) -> T*
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        raise_invalid_entity_exception();
        return nullptr;
    }
    auto comp = e.try_get<T>();

    if(!comp)
    {
        raise_missing_component_exception<T>();
        return nullptr;
    }

    return comp;
}

void internal_m2n_load_scene(const std::string& key)
{
    auto& ctx = engine::context();
    auto& ec = ctx.get_cached<ecs>();
    auto& am = ctx.get_cached<asset_manager>();

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
    auto& ec = ctx.get_cached<ecs>();

    auto e = ec.get_scene().create_entity(tag);

    return e.entity();
}

auto internal_m2n_create_entity_from_prefab_uid(const hpp::uuid& uid) -> entt::entity
{
    auto& ctx = engine::context();
    auto& ec = ctx.get_cached<ecs>();
    auto& am = ctx.get_cached<asset_manager>();

    auto pfb = am.get_asset<prefab>(uid);
    auto e = ec.get_scene().instantiate(pfb);

    return e.entity();
}

auto internal_m2n_create_entity_from_prefab_key(const std::string& key) -> entt::entity
{
    auto& ctx = engine::context();
    auto& ec = ctx.get_cached<ecs>();
    auto& am = ctx.get_cached<asset_manager>();

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
        auto& ec = ctx.get_cached<ecs>();

        auto cloned = ec.get_scene().clone_entity(e);
        return cloned.entity();
    }

    entt::handle invalid;
    return invalid.entity();
}

auto internal_m2n_destroy_entity_immediate(entt::entity id) -> bool
{
    auto e = get_entity_from_id(id);
    if(e)
    {
        e.destroy();
        return true;
    }
    return false;
}

auto internal_m2n_destroy_entity(entt::entity id, float seconds) -> bool
{
    seconds = std::max(0.0001f, seconds);

    delta_t secs(seconds);
    auto dur = std::chrono::duration_cast<tweeny::duration_t>(secs);

    auto tween = tweeny::delay(dur);
    tween.on_end.connect(
        [id]()
        {
            internal_m2n_destroy_entity_immediate(id);
        });

    tweeny::tween_scope_policy policy{};
    policy.scope = "script";
    tweeny::start(tween, policy);

    return true;
}

auto internal_m2n_is_entity_valid(entt::entity id) -> bool
{
    auto e = get_entity_from_id(id);
    bool valid = e.valid();
    return valid;
}

auto internal_m2n_find_entity_by_tag(const std::string& tag) -> uint32_t
{
    auto& ctx = engine::context();
    auto& ec = ctx.get_cached<ecs>();
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
    std::function<bool(const std::string& type_name, entt::handle e)> add_native;
    std::function<bool(const std::string& type_name, entt::handle e)> has_native;
    std::function<bool(const std::string& type_name, entt::handle e)> remove_native;

    static auto get_registry() -> std::unordered_map<std::string, native_comp_lut>&
    {
        static std::unordered_map<std::string, native_comp_lut> lut;
        return lut;
    }

    static auto get_action_table(const std::string& type_name) -> const native_comp_lut&
    {
        const auto& registry = get_registry();
        auto it = registry.find(type_name);
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
        lut.add_native = [name](const std::string& type_name, entt::handle e)
        {
            if(type_name == name)
            {
                auto& native = e.get_or_emplace<T>();
                return true;
            }

            return false;
        };

        lut.has_native = [name](const std::string& type_name, entt::handle e)
        {
            if(type_name == name)
            {
                return e.all_of<T>();
            }

            return false;
        };

        lut.remove_native = [name](const std::string& type_name, entt::handle e)
        {
            if(type_name == name)
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
    native_comp_lut::register_native_component<transform_component>("TransformComponent");
    native_comp_lut::register_native_component<id_component>("IdComponent");
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
    // TODO OPTIMIZE

    const auto& type_name = type.get_name();
    bool add = false;

    const auto& lut = native_comp_lut::get_action_table(type_name);
    if(lut.is_valid())
    {
        add = lut.add_native(type_name, e);
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

auto internal_get_native_component_impl(const mono::mono_type& type,
                                        entt::handle e,
                                        script_component& script_comp,
                                        bool exists) -> mono::mono_object
{
    auto comp = script_comp.get_native_component(type);
    if(exists)
    {
        if(!comp.scoped)
        {
            comp = script_comp.add_native_component(type);
        }
        return static_cast<mono::mono_object&>(comp.scoped->object);
    }

    if(comp.scoped)
    {
        script_comp.remove_native_component(comp.scoped->object);
    }

    return {};
}

auto internal_get_native_component(const mono::mono_type& type, entt::handle e, script_component& script_comp)
    -> mono::mono_object
{
    const auto& type_name = type.get_name();

    // TODO OPTIMIZE
    bool native = false;
    bool has = false;

    const auto& lut = native_comp_lut::get_action_table(type_name);
    if(lut.is_valid())
    {
        has = lut.has_native(type_name, e);
        native = true;
    }

    if(native)
    {
        return internal_get_native_component_impl(type, e, script_comp, has);
    }

    return {};
}

auto internal_remove_native_component(const mono::mono_object& obj, entt::handle e, script_component& script_comp)
    -> bool
{
    const auto& type = obj.get_type();
    const auto& type_name = type.get_name();

    // TODO OPTIMIZE

    bool removed = false;
    const auto& lut = native_comp_lut::get_action_table(type_name);
    if(lut.is_valid())
    {
        removed = lut.remove_native(type_name, e);
    }

    if(removed)
    {
        return script_comp.remove_native_component(obj);
    }

    return false;
}

auto internal_remove_native_component(const mono::mono_type& type, entt::handle e, script_component& script_comp)
    -> bool
{
    const auto& type_name = type.get_name();

    // TODO OPTIMIZE

    bool removed = false;
    const auto& lut = native_comp_lut::get_action_table(type_name);
    if(lut.is_valid())
    {
        removed = lut.remove_native(type_name, e);
    }

    if(removed)
    {
        return script_comp.remove_native_component(type);
    }

    return false;
}

auto internal_m2n_add_component(entt::entity id, const mono::mono_type& type) -> mono::mono_object
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        raise_invalid_entity_exception();
        return {};
    }
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
    if(!e)
    {
        raise_invalid_entity_exception();
        return {};
    }

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

auto internal_m2n_get_components(entt::entity id, const mono::mono_type& type) -> std::vector<mono::mono_object>
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        raise_invalid_entity_exception();
        return {};
    }

    auto& script_comp = e.get_or_emplace<script_component>();

    if(auto native_comp = internal_get_native_component(type, e, script_comp))
    {
        return {native_comp};
    }

    return script_comp.get_script_components(type);
}

auto internal_m2n_get_transform_component(entt::entity id, const mono::mono_type& type) -> mono::mono_object
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        raise_invalid_entity_exception();
        return {};
    }

    auto& script_comp = e.get_or_emplace<script_component>();
    return internal_get_native_component_impl(type, e, script_comp, true);
}

auto internal_m2n_get_tag(entt::entity id) -> const std::string&
{
    if(auto comp = safe_get_component<tag_component>(id))
    {
        return comp->tag;
    }

    static const std::string empty;
    return empty;
}

void internal_m2n_set_tag(entt::entity id, const std::string& tag)
{
    if(auto comp = safe_get_component<tag_component>(id))
    {
        comp->tag = tag;
    }
}

auto internal_m2n_has_component(entt::entity id, const mono::mono_type& type) -> bool
{
    auto comp = internal_m2n_get_component(id, type);

    return comp.valid();
}

auto internal_m2n_remove_component_instance(entt::entity id, const mono::mono_object& comp) -> bool
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        raise_invalid_entity_exception();
        return false;
    }
    auto& script_comp = e.get_or_emplace<script_component>();

    if(internal_remove_native_component(comp, e, script_comp))
    {
        return true;
    }

    return script_comp.remove_script_component(comp);
}

auto internal_m2n_remove_component(entt::entity id, const mono::mono_type& type) -> bool
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        raise_invalid_entity_exception();
        return false;
    }
    auto& script_comp = e.get_or_emplace<script_component>();

    if(internal_remove_native_component(type, e, script_comp))
    {
        return true;
    }

    return script_comp.remove_script_component(type);
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
auto internal_m2n_get_children(entt::entity id) -> std::vector<entt::entity>
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        const auto& children = comp->get_children();
        std::vector<entt::entity> children_id;
        children_id.reserve(children.size());
        for(const auto& child : children)
        {
            children_id.emplace_back(child.entity());
        }
        return children_id;
    }

    return {};
}

auto internal_m2n_get_parent(entt::entity id) -> entt::entity
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_parent().entity();
    }

    return {};
}

void internal_m2n_set_parent(entt::entity id, entt::entity new_parent, bool global_stays)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        auto parent = get_entity_from_id(new_parent);
        comp->set_parent(parent, global_stays);
    }
}

auto internal_m2n_get_position_global(entt::entity id) -> math::vec3
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_position_global();
    }

    return {};
}

void internal_m2n_set_position_global(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_position_global(value);
    }
}

void internal_m2n_move_by_global(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->move_by_global(value);
    }
}

auto internal_m2n_get_position_local(entt::entity id) -> math::vec3
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_position_local();
    }

    return {};
}

void internal_m2n_set_position_local(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_position_local(value);
    }
}

void internal_m2n_move_by_local(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->move_by_local(value);
    }
}

//--------------------------------------------------
auto internal_m2n_get_rotation_euler_global(entt::entity id) -> math::vec3
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_rotation_euler_global();
    }

    return {};
}

void internal_m2n_rotate_by_euler_global(entt::entity id, const math::vec3& amount)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->rotate_by_euler_global(amount);
    }
}

void internal_m2n_rotate_axis_global(entt::entity id, float degrees, const math::vec3& axis)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->rotate_axis_global(degrees, axis);
    }
}

void internal_m2n_look_at(entt::entity id, const math::vec3& point, const math::vec3& up)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->look_at(point, up);
    }
}

void internal_m2n_set_rotation_euler_global(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_rotation_euler_global(value);
    }
}

auto internal_m2n_get_rotation_euler_local(entt::entity id) -> math::vec3
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_rotation_euler_local();
    }

    return {};
}

void internal_m2n_set_rotation_euler_local(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_rotation_euler_local(value);
    }
}

void internal_m2n_rotate_by_euler_local(entt::entity id, const math::vec3& amount)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->rotate_by_euler_local(amount);
    }
}

auto internal_m2n_get_rotation_global(entt::entity id) -> math::quat
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_rotation_global();
    }

    return {};
}

void internal_m2n_set_rotation_global(entt::entity id, const math::quat& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_rotation_global(value);
    }
}

void internal_m2n_rotate_by_global(entt::entity id, const math::quat& amount)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->rotate_by_global(amount);
    }
}

auto internal_m2n_get_rotation_local(entt::entity id) -> math::quat
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_rotation_local();
    }

    return {};
}

void internal_m2n_set_rotation_local(entt::entity id, const math::quat& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_rotation_local(value);
    }
}

void internal_m2n_rotate_by_local(entt::entity id, const math::quat& amount)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->rotate_by_local(amount);
    }
}

//--------------------------------------------------
auto internal_m2n_get_scale_global(entt::entity id) -> math::vec3
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_scale_global();
    }

    return {};
}

void internal_m2n_set_scale_global(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_scale_global(value);
    }
}

void internal_m2n_scale_by_global(entt::entity id, const math::vec3& amount)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->scale_by_global(amount);
    }
}

auto internal_m2n_get_scale_local(entt::entity id) -> math::vec3
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_scale_local();
    }

    return {};
}

void internal_m2n_set_scale_local(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_scale_local(value);
    }
}

void internal_m2n_scale_by_local(entt::entity id, const math::vec3& amount)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->scale_by_local(amount);
    }
}

//--------------------------------------------------
auto internal_m2n_get_skew_global(entt::entity id) -> math::vec3
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_skew_global();
    }

    return {};
}

void internal_m2n_setl_skew_globa(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_skew_global(value);
    }
}

auto internal_m2n_get_skew_local(entt::entity id) -> math::vec3
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        return comp->get_skew_local();
    }

    return {};
}

void internal_m2n_set_skew_local(entt::entity id, const math::vec3& value)
{
    if(auto comp = safe_get_component<transform_component>(id))
    {
        comp->set_skew_local(value);
    }
}

//------------------------------

void internal_m2n_apply_explosion_force(entt::entity id,
                                        float explosion_force,
                                        const math::vec3& explosion_position,
                                        float explosion_radius,
                                        float upwards_modifier,
                                        force_mode mode)
{
    if(auto comp = safe_get_component<physics_component>(id))
    {
        comp->apply_explosion_force(explosion_force, explosion_position, explosion_radius, upwards_modifier, mode);
    }
}
void internal_m2n_apply_force(entt::entity id, const math::vec3& value, force_mode mode)
{
    if(auto comp = safe_get_component<physics_component>(id))
    {
        comp->apply_force(value, mode);
    }
}

void internal_m2n_apply_torque(entt::entity id, const math::vec3& value, force_mode mode)
{
    if(auto comp = safe_get_component<physics_component>(id))
    {
        comp->apply_torque(value, mode);
    }
}

//------------------------------

void internal_m2n_animation_blend(entt::entity id, hpp::uuid guid, float seconds)
{
    if(auto comp = safe_get_component<animation_component>(id))
    {
        auto& ctx = engine::context();
        auto& am = ctx.get_cached<asset_manager>();

        auto asset = am.get_asset<animation_clip>(guid);
        comp->get_player().blend_to(asset, animation_player::seconds_t(seconds));
    }
}

void internal_m2n_animation_play(entt::entity id)
{
    if(auto comp = safe_get_component<animation_component>(id))
    {
        comp->get_player().play();
    }
}

void internal_m2n_animation_pause(entt::entity id)
{
    if(auto comp = safe_get_component<animation_component>(id))
    {
        comp->get_player().pause();
    }
}

void internal_m2n_animation_resume(entt::entity id)
{
    if(auto comp = safe_get_component<animation_component>(id))
    {
        comp->get_player().resume();
    }
}

void internal_m2n_animation_stop(entt::entity id)
{
    if(auto comp = safe_get_component<animation_component>(id))
    {
        comp->get_player().stop();
    }
}

//------------------------------
auto internal_m2n_camera_viewport_to_ray(entt::entity id,
                                         const math::vec2& origin,
                                         mono::managed_interface::ray* managed_ray) -> bool
{
    if(auto comp = safe_get_component<camera_component>(id))
    {
        math::vec3 ray_origin{};
        math::vec3 ray_dir{};
        bool result = comp->get_camera().viewport_to_ray(origin, ray_origin, ray_dir);
        if(result)
        {
            using converter = mono::managed_interface::converter;
            managed_ray->origin = converter::convert<math::vec3, mono::managed_interface::vector3>(ray_origin);
            managed_ray->direction = converter::convert<math::vec3, mono::managed_interface::vector3>(ray_dir);
        }
        return result;
    }

    return false;
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
    auto& am = ctx.get_cached<asset_manager>();

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
    auto& am = ctx.get_cached<asset_manager>();

    if(type.get_name() == "Prefab")
    {
        auto asset = am.get_asset<prefab>(key);
        return asset.uid();
    }

    return {};
}

auto internal_m2n_audio_clip_get_length(const hpp::uuid& uid) -> float
{
    auto& ctx = engine::context();
    auto& am = ctx.get_cached<asset_manager>();

    auto asset = am.get_asset<audio_clip>(uid);

    if(asset.is_valid())
    {
        float secs = asset.get()->get_info().duration.count();
        return secs;
    }

    return 0.0f;
}

auto m2n_test_uuid(const hpp::uuid& uid) -> hpp::uuid
{
    APPLOG_INFO("{}:: From C# {}", __func__, hpp::to_string(uid));

    auto newuid = generate_uuid();
    APPLOG_INFO("{}:: New C++ {}", __func__, hpp::to_string(newuid));

    return newuid;
}

auto internal_m2n_input_get_analog_value(const std::string& name) -> float
{
    auto& ctx = engine::context();
    auto& input = ctx.get_cached<input_system>();
    return input.get_analog_value(name);
}

auto internal_m2n_input_get_digital_value(const std::string& name) -> bool
{
    auto& ctx = engine::context();
    auto& input = ctx.get_cached<input_system>();
    return input.get_digital_value(name);
}

auto internal_m2n_input_is_pressed(const std::string& name) -> bool
{
    auto& ctx = engine::context();
    auto& input = ctx.get_cached<input_system>();
    return input.is_pressed(name);
}

auto internal_m2n_input_is_released(const std::string& name) -> bool
{
    auto& ctx = engine::context();
    auto& input = ctx.get_cached<input_system>();
    return input.is_released(name);
}

auto internal_m2n_input_is_down(const std::string& name) -> bool
{
    auto& ctx = engine::context();
    auto& input = ctx.get_cached<input_system>();
    return input.is_down(name);
}

auto internal_m2n_input_get_mouse_position() -> math::vec2
{
    auto& ctx = engine::context();
    auto& input = ctx.get_cached<input_system>();
    auto coord = input.manager.get_mouse().get_position();
    return {coord.x, coord.y};
}

//-------------------------------------------------

auto internal_m2n_physics_raycast(mono::managed_interface::raycast_hit* hit,
                                  const math::vec3& origin,
                                  const math::vec3& direction,
                                  float max_distance,
                                  int layer_mask,
                                  bool query_sensors) -> bool
{
    auto& ctx = engine::context();
    auto& physics = ctx.get_cached<physics_system>();

    auto ray_hit = physics.ray_cast(origin, direction, max_distance, layer_mask, query_sensors);

    using converter = mono::managed_interface::converter;

    if(ray_hit)
    {
        hit->entity = ray_hit->entity;
        hit->point = converter::convert<math::vec3, mono::managed_interface::vector3>(ray_hit->point);
        hit->normal = converter::convert<math::vec3, mono::managed_interface::vector3>(ray_hit->normal);
        hit->distance = ray_hit->distance;
    }

    return ray_hit.has_value();
}

auto internal_m2n_physics_raycast_all(const math::vec3& origin,
                                      const math::vec3& direction,
                                      float max_distance,
                                      int layer_mask,
                                      bool query_sensors) -> std::vector<mono::managed_interface::raycast_hit>
{
    auto& ctx = engine::context();
    auto& physics = ctx.get_cached<physics_system>();

    auto ray_hits = physics.ray_cast_all(origin, direction, max_distance, layer_mask, query_sensors);

    std::vector<mono::managed_interface::raycast_hit> hits;

    using converter = mono::managed_interface::converter;
    for(const auto& ray_hit : ray_hits)
    {
        auto& hit = hits.emplace_back();
        hit.entity = ray_hit.entity;
        hit.point = converter::convert<math::vec3, mono::managed_interface::vector3>(ray_hit.point);
        hit.normal = converter::convert<math::vec3, mono::managed_interface::vector3>(ray_hit.normal);
        hit.distance = ray_hit.distance;
    }

    return hits;
}

//-------------------------------------------------

auto internal_m2n_audio_source_get_loop(entt::entity id) -> bool
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->is_looping();
    }

    return {};
}

void internal_m2n_audio_source_set_loop(entt::entity id, bool loop)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->set_loop(loop);
    }
}

auto internal_m2n_audio_source_get_volume(entt::entity id) -> float
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->get_volume();
    }

    return {};
}

void internal_m2n_audio_source_set_volume(entt::entity id, float volume)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->set_volume(volume);
    }
}

auto internal_m2n_audio_source_get_pitch(entt::entity id) -> float
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->get_pitch();
    }

    return {};
}

void internal_m2n_audio_source_set_pitch(entt::entity id, float pitch)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->set_pitch(pitch);
    }
}

auto internal_m2n_audio_source_get_volume_rolloff(entt::entity id) -> float
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->get_volume_rolloff();
    }

    return {};
}

void internal_m2n_audio_source_set_volume_rolloff(entt::entity id, float rolloff)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->set_volume_rolloff(rolloff);
    }
}

auto internal_m2n_audio_source_get_min_distance(entt::entity id) -> float
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->get_range().min;
    }

    return {};
}

void internal_m2n_audio_source_set_min_distance(entt::entity id, float distance)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        auto range = comp->get_range();
        range.min = distance;
        comp->set_range(range);
    }
}

auto internal_m2n_audio_source_get_max_distance(entt::entity id) -> float
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->get_range().max;
    }

    return {};
}

void internal_m2n_audio_source_set_max_distance(entt::entity id, float distance)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        auto range = comp->get_range();
        range.max = distance;
        comp->set_range(range);
    }
}

auto internal_m2n_audio_source_get_mute(entt::entity id) -> bool
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->is_muted();
    }

    return {};
}

void internal_m2n_audio_source_set_mute(entt::entity id, bool mute)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->set_mute(mute);
    }
}

auto internal_m2n_audio_source_get_time(entt::entity id) -> float
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return float(comp->get_playback_position().count());
    }

    return {};
}

void internal_m2n_audio_source_set_time(entt::entity id, float seconds)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->set_playback_position(audio::duration_t(seconds));
    }
}

auto internal_m2n_audio_source_is_playing(entt::entity id) -> bool
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->is_playing();
    }

    return {};
}

auto internal_m2n_audio_source_is_paused(entt::entity id) -> bool
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->is_paused();
    }

    return {};
}

void internal_m2n_audio_source_play(entt::entity id)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->play();
    }
}

void internal_m2n_audio_source_stop(entt::entity id)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->stop();
    }
}

void internal_m2n_audio_source_pause(entt::entity id)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->pause();
    }
}

void internal_m2n_audio_source_resume(entt::entity id)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        comp->resume();
    }
}

auto internal_m2n_audio_source_get_audio_clip(entt::entity id) -> hpp::uuid
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        return comp->get_clip().uid();
    }

    return {};
}

void internal_m2n_audio_source_set_audio_clip(entt::entity id, hpp::uuid uid)
{
    if(auto comp = safe_get_component<audio_source_component>(id))
    {
        auto& ctx = engine::context();
        auto& am = ctx.get_cached<asset_manager>();

        auto asset = am.get_asset<audio_clip>(uid);
        comp->set_clip(asset);
    }
}

//--------------------------------------------------
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
        reg.add_internal_call("internal_m2n_clone_entity", internal_call(internal_m2n_clone_entity));
        reg.add_internal_call("internal_m2n_destroy_entity", internal_call(internal_m2n_destroy_entity));
        reg.add_internal_call("internal_m2n_destroy_entity_immediate",
                              internal_call(internal_m2n_destroy_entity_immediate));

        reg.add_internal_call("internal_m2n_is_entity_valid", internal_call(internal_m2n_is_entity_valid));
        reg.add_internal_call("internal_m2n_find_entity_by_tag", internal_call(internal_m2n_find_entity_by_tag));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.Entity");
        reg.add_internal_call("internal_m2n_add_component", internal_call(internal_m2n_add_component));
        reg.add_internal_call("internal_m2n_get_component", internal_call(internal_m2n_get_component));
        reg.add_internal_call("internal_m2n_has_component", internal_call(internal_m2n_has_component));
        reg.add_internal_call("internal_m2n_get_components", internal_call(internal_m2n_get_components));
        reg.add_internal_call("internal_m2n_remove_component_instance",
                              internal_call(internal_m2n_remove_component_instance));

        reg.add_internal_call("internal_m2n_remove_component", internal_call(internal_m2n_remove_component));
        reg.add_internal_call("internal_m2n_get_transform_component",
                              internal_call(internal_m2n_get_transform_component));
        reg.add_internal_call("internal_m2n_get_tag", internal_call(internal_m2n_get_tag));
        reg.add_internal_call("internal_m2n_set_tag", internal_call(internal_m2n_set_tag));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.TransformComponent");
        reg.add_internal_call("internal_m2n_get_children", internal_call(internal_m2n_get_children));
        reg.add_internal_call("internal_m2n_get_parent", internal_call(internal_m2n_get_parent));

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

        // Skew
        reg.add_internal_call("internal_m2n_get_skew_global", internal_call(internal_m2n_get_skew_global));
        reg.add_internal_call("internal_m2n_set_skew_globa", internal_call(internal_m2n_setl_skew_globa));
        reg.add_internal_call("internal_m2n_get_skew_local", internal_call(internal_m2n_get_skew_local));
        reg.add_internal_call("internal_m2n_set_skew_local", internal_call(internal_m2n_set_skew_local));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.PhysicsComponent");
        reg.add_internal_call("internal_m2n_apply_explosion_force", internal_call(internal_m2n_apply_explosion_force));
        reg.add_internal_call("internal_m2n_apply_force", internal_call(internal_m2n_apply_force));
        reg.add_internal_call("internal_m2n_apply_torque", internal_call(internal_m2n_apply_torque));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.AnimationComponent");
        reg.add_internal_call("internal_m2n_animation_blend", internal_call(internal_m2n_animation_blend));
        reg.add_internal_call("internal_m2n_animation_play", internal_call(internal_m2n_animation_play));
        reg.add_internal_call("internal_m2n_animation_pause", internal_call(internal_m2n_animation_pause));
        reg.add_internal_call("internal_m2n_animation_resume", internal_call(internal_m2n_animation_resume));
        reg.add_internal_call("internal_m2n_animation_stop", internal_call(internal_m2n_animation_stop));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.CameraComponent");
        reg.add_internal_call("internal_m2n_camera_viewport_to_ray",
                              internal_call(internal_m2n_camera_viewport_to_ray));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.Assets");
        reg.add_internal_call("internal_m2n_get_asset_by_uuid", internal_call(internal_m2n_get_asset_by_uuid));
        reg.add_internal_call("internal_m2n_get_asset_by_key", internal_call(internal_m2n_get_asset_by_key));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.AudioClip");
        reg.add_internal_call("internal_m2n_audio_clip_get_length", internal_call(internal_m2n_audio_clip_get_length));
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

    {
        auto reg = mono::internal_call_registry("Ace.Core.Input");
        reg.add_internal_call("internal_m2n_input_get_analog_value",
                              internal_call(internal_m2n_input_get_analog_value));
        reg.add_internal_call("internal_m2n_input_get_digital_value",
                              internal_call(internal_m2n_input_get_analog_value));
        reg.add_internal_call("internal_m2n_input_is_pressed", internal_call(internal_m2n_input_is_pressed));
        reg.add_internal_call("internal_m2n_input_is_released", internal_call(internal_m2n_input_is_released));
        reg.add_internal_call("internal_m2n_input_is_down", internal_call(internal_m2n_input_is_down));
        reg.add_internal_call("internal_m2n_input_get_mouse_position",
                              internal_call(internal_m2n_input_get_mouse_position));
    }

    {
        auto reg = mono::internal_call_registry("Ace.Core.Physics");
        reg.add_internal_call("internal_m2n_physics_raycast", internal_call(internal_m2n_physics_raycast));
        reg.add_internal_call("internal_m2n_physics_raycast_all", internal_call(internal_m2n_physics_raycast_all));
    }
    {
        auto reg = mono::internal_call_registry("Ace.Core.AudioSourceComponent");
        reg.add_internal_call("internal_m2n_audio_source_get_loop", internal_call(internal_m2n_audio_source_get_loop));
        reg.add_internal_call("internal_m2n_audio_source_set_loop", internal_call(internal_m2n_audio_source_set_loop));
        reg.add_internal_call("internal_m2n_audio_source_get_volume",
                              internal_call(internal_m2n_audio_source_get_volume));
        reg.add_internal_call("internal_m2n_audio_source_set_volume",
                              internal_call(internal_m2n_audio_source_set_volume));
        reg.add_internal_call("internal_m2n_audio_source_get_pitch",
                              internal_call(internal_m2n_audio_source_get_pitch));
        reg.add_internal_call("internal_m2n_audio_source_set_pitch",
                              internal_call(internal_m2n_audio_source_set_pitch));
        reg.add_internal_call("internal_m2n_audio_source_get_volume_rolloff",
                              internal_call(internal_m2n_audio_source_get_volume_rolloff));
        reg.add_internal_call("internal_m2n_audio_source_set_volume_rolloff",
                              internal_call(internal_m2n_audio_source_set_volume_rolloff));
        reg.add_internal_call("internal_m2n_audio_source_get_min_distance",
                              internal_call(internal_m2n_audio_source_get_min_distance));
        reg.add_internal_call("internal_m2n_audio_source_set_min_distance",
                              internal_call(internal_m2n_audio_source_set_min_distance));
        reg.add_internal_call("internal_m2n_audio_source_get_max_distance",
                              internal_call(internal_m2n_audio_source_get_max_distance));
        reg.add_internal_call("internal_m2n_audio_source_set_max_distance",
                              internal_call(internal_m2n_audio_source_set_max_distance));
        reg.add_internal_call("internal_m2n_audio_source_get_mute", internal_call(internal_m2n_audio_source_get_mute));

        reg.add_internal_call("internal_m2n_audio_source_set_mute", internal_call(internal_m2n_audio_source_set_mute));

        reg.add_internal_call("internal_m2n_audio_source_is_playing",
                              internal_call(internal_m2n_audio_source_is_playing));
        reg.add_internal_call("internal_m2n_audio_source_is_paused",
                              internal_call(internal_m2n_audio_source_is_paused));
        reg.add_internal_call("internal_m2n_audio_source_play", internal_call(internal_m2n_audio_source_play));
        reg.add_internal_call("internal_m2n_audio_source_stop", internal_call(internal_m2n_audio_source_stop));

        reg.add_internal_call("internal_m2n_audio_source_pause", internal_call(internal_m2n_audio_source_pause));
        reg.add_internal_call("internal_m2n_audio_source_resume", internal_call(internal_m2n_audio_source_resume));
        reg.add_internal_call("internal_m2n_audio_source_get_audio_clip",
                              internal_call(internal_m2n_audio_source_get_audio_clip));
        reg.add_internal_call("internal_m2n_audio_source_set_audio_clip",
                              internal_call(internal_m2n_audio_source_set_audio_clip));
    }
    // mono::managed_interface::init(assembly);

    return true;
}

} // namespace ace
