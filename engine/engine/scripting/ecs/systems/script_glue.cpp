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
#include <engine/ecs/components/id_component.h>
#include <engine/ecs/components/transform_component.h>

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
} // namespace managed_interface

register_basic_mono_converter_for_pod(math::vec2, managed_interface::vector2);
register_basic_mono_converter_for_pod(math::vec3, managed_interface::vector3);
register_basic_mono_converter_for_pod(math::vec4, managed_interface::vector4);

} // namespace mono

namespace ace
{
namespace
{

auto get_entity_from_id(uint32_t id) -> entt::handle
{
    if(id == 0)
    {
        return {};
    }
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    return ec.get_scene().create_entity(entt::entity(id));
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

auto internal_m2n_create_entity(const std::string& tag) -> uint32_t
{
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    auto e = ec.get_scene().create_entity(tag);

    return static_cast<uint32_t>(e.entity());
}

auto internal_m2n_destroy_entity(uint32_t id) -> bool
{
    auto e = get_entity_from_id(id);
    if(e)
    {
        e.destroy();
        return true;
    }
    return false;
}

auto internal_m2n_is_entity_valid(uint32_t id) -> bool
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

auto internal_add_native_component(const mono::mono_type& type, entt::handle e, script_component& script_comp)
    -> mono::mono_object
{
    bool add = false;
    if(type.get_name() == "TransformComponent")
    {
        auto& native = e.get_or_emplace<transform_component>();
        add = true;
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
    if(type.get_name() == "TransformComponent")
    {
        has = e.all_of<transform_component>();
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
    if(obj.get_type().get_name() == "TransformComponent")
    {
        e.remove<transform_component>();
        return script_comp.remove_native_component(obj);
    }

    return false;
}

auto internal_m2n_add_component(uint32_t id, const mono::mono_type& type) -> mono::mono_object
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

auto internal_m2n_get_component(uint32_t id, const mono::mono_type& type) -> mono::mono_object
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

auto internal_m2n_has_component(uint32_t id, const mono::mono_type& type) -> bool
{
    auto comp = internal_m2n_get_component(id, type);

    return comp.valid();
}

auto internal_m2n_remove_component(uint32_t id, const mono::mono_object& comp) -> bool
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

//--------------------------------------------
auto internal_m2n_get_position_global(uint32_t id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_position_global();
}

void internal_m2n_set_position_global(uint32_t id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_position_global(value);
}

auto internal_m2n_get_position_local(uint32_t id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_position_local();
}

void internal_m2n_set_position_local(uint32_t id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_position_local(value);
}

//--------------------------------------------------
auto internal_m2n_get_rotation_euler_global(uint32_t id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_rotation_euler_global();
}

void internal_m2n_set_rotation_euler_global(uint32_t id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_rotation_euler_global(value);
}

auto internal_m2n_get_rotation_euler_local(uint32_t id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_rotation_euler_local();
}

void internal_m2n_set_rotation_euler_local(uint32_t id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_rotation_euler_local(value);
}

//--------------------------------------------------
auto internal_m2n_get_scale_global(uint32_t id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_scale_global();
}

void internal_m2n_set_scale_global(uint32_t id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_scale_global(value);
}

auto internal_m2n_get_scale_local(uint32_t id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_scale_local();
}

void internal_m2n_set_scale_local(uint32_t id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_scale_local(value);
}

//--------------------------------------------------
auto internal_m2n_get_skew_global(uint32_t id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_skew_global();
}

void internal_m2n_setl_skew_globa(uint32_t id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_skew_global(value);
}

auto internal_m2n_get_skew_local(uint32_t id) -> math::vec3
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return {};
    }

    const auto& transform_comp = e.get<transform_component>();
    return transform_comp.get_skew_local();
}

void internal_m2n_set_skew_local(uint32_t id, const math::vec3& value)
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return;
    }

    auto& transform_comp = e.get<transform_component>();
    transform_comp.set_skew_local(value);
}

} // namespace

auto script_system::bind_internal_calls(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    mono::add_internal_call("Ace.Core.Log::internal_m2n_log_trace", internal_call(internal_m2n_log_trace));
    mono::add_internal_call("Ace.Core.Log::internal_m2n_log_info", internal_call(internal_m2n_log_info));
    mono::add_internal_call("Ace.Core.Log::internal_m2n_log_warning", internal_call(internal_m2n_log_warning));
    mono::add_internal_call("Ace.Core.Log::internal_m2n_log_error", internal_call(internal_m2n_log_error));

    mono::add_internal_call("Ace.Core.Scene::internal_m2n_load_scene", internal_call(internal_m2n_load_scene));
    mono::add_internal_call("Ace.Core.Scene::internal_m2n_create_scene", internal_call(internal_m2n_create_scene));
    mono::add_internal_call("Ace.Core.Scene::internal_m2n_destroy_scene", internal_call(internal_m2n_destroy_scene));
    mono::add_internal_call("Ace.Core.Scene::internal_m2n_create_entity", internal_call(internal_m2n_create_entity));
    mono::add_internal_call("Ace.Core.Scene::internal_m2n_destroy_entity", internal_call(internal_m2n_destroy_entity));
    mono::add_internal_call("Ace.Core.Scene::internal_m2n_is_entity_valid",
                            internal_call(internal_m2n_is_entity_valid));
    mono::add_internal_call("Ace.Core.Scene::internal_m2n_find_entity_by_tag",
                            internal_call(internal_m2n_find_entity_by_tag));

    mono::add_internal_call("Ace.Core.Entity::internal_m2n_add_component", internal_call(internal_m2n_add_component));
    mono::add_internal_call("Ace.Core.Entity::internal_m2n_get_component", internal_call(internal_m2n_get_component));
    mono::add_internal_call("Ace.Core.Entity::internal_m2n_has_component", internal_call(internal_m2n_has_component));
    mono::add_internal_call("Ace.Core.Entity::internal_m2n_remove_component",
                            internal_call(internal_m2n_remove_component));

    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_get_position_global",
                            internal_call(internal_m2n_get_position_global));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_set_position_global",
                            internal_call(internal_m2n_set_position_global));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_get_position_local",
                            internal_call(internal_m2n_get_position_local));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_set_position_local",
                            internal_call(internal_m2n_set_position_local));

    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_get_rotation_euler_global",
                            internal_call(internal_m2n_get_rotation_euler_global));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_set_rotation_euler_global",
                            internal_call(internal_m2n_set_rotation_euler_global));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_get_rotation_euler_local",
                            internal_call(internal_m2n_get_rotation_euler_local));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_set_rotation_euler_local",
                            internal_call(internal_m2n_set_rotation_euler_local));

    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_get_scale_global",
                            internal_call(internal_m2n_get_scale_global));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_set_scale_global",
                            internal_call(internal_m2n_set_scale_global));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_get_scale_local",
                            internal_call(internal_m2n_get_scale_local));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_set_scale_local",
                            internal_call(internal_m2n_set_scale_local));

    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_get_skew_global",
                            internal_call(internal_m2n_get_skew_global));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_setl_skew_globa",
                            internal_call(internal_m2n_setl_skew_globa));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_get_skew_local",
                            internal_call(internal_m2n_get_skew_local));
    mono::add_internal_call("Ace.Core.TransformComponent::internal_m2n_set_skew_local",
                            internal_call(internal_m2n_set_skew_local));
    // mono::managed_interface::init(assembly);

    return true;
}

} // namespace ace
