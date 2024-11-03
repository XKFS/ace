#include "script_system.h"
#include <engine/ecs/ecs.h>
#include <engine/events.h>

#include <engine/engine.h>
#include <monopp/mono_exception.h>
#include <monopp/mono_internal_call.h>
#include <monopp/mono_jit.h>
#include <monort/monort.h>

#include <engine/scripting/ecs/components/script_component.h>

#include <filesystem/filesystem.h>
#include <logging/logging.h>

namespace ace
{
namespace
{

auto get_entity_from_id(uint32_t id) -> entt::handle
{
    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    return ec.get_scene().create_entity(entt::entity(id));
}

void Internal_CreateScene(const mono::mono_object& this_ptr)
{
    mono::ignore(this_ptr);
    // std::cout << "FROM C++ : MyObject created." << std::endl;
}

void Internal_DestroyScene(const mono::mono_object& this_ptr)
{
    mono::ignore(this_ptr);
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;
}

uint32_t Internal_CreateEntity(const std::string& tag)
{
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;

    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    auto e = ec.get_scene().create_entity(tag);
    return static_cast<uint32_t>(e.entity());
}

bool Internal_DestroyEntity(uint32_t id)
{
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;
    auto e = get_entity_from_id(id);
    if(e)
    {
        e.destroy();
        return true;
    }
    return false;
}

bool Internal_IsEntityValid(uint32_t id)
{
    entt::handle h;
    auto e = get_entity_from_id(id);
    return e.valid();
}

auto Internal_AddComponent(uint32_t id, const mono::mono_type& type) -> mono::mono_object
{
    auto e = get_entity_from_id(id);

    // auto& ctx = engine::context();
    // auto& ec = ctx.get<script_system>();

    auto base_type = type.get_base_type();
    if(base_type.get_name() == "Component")
    {
        return {};
    }

    auto& script_comp = e.get_or_emplace<script_component>();
    auto component = script_comp.add_script_component(type);
    return static_cast<mono::mono_object&>(component.scoped->object);
}

auto Internal_GetComponent(uint32_t id, const mono::mono_type& type) -> mono::mono_object
{
    auto e = get_entity_from_id(id);

    auto base_type = type.get_base_type();
    if(base_type.get_name() == "Component")
    {
        return {};
    }

    auto& script_comp = e.get_or_emplace<script_component>();
    auto component = script_comp.get_script_component(type);

    if(component.scoped)
    {
        return static_cast<mono::mono_object&>(component.scoped->object);
    }

    return {};
}

auto Internal_RemoveComponent(uint32_t id, const mono::mono_object& comp) -> bool
{
    auto e = get_entity_from_id(id);
    if(!e)
    {
        return false;
    }

    auto& script_comp = e.get_or_emplace<script_component>();
    bool result = script_comp.remove_script_component(comp);
    script_comp.process_pending_deletions();
    return result;
}

void Internal_LogTrace(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_TRACE_LOC(file.c_str(), line, func.c_str(), message);
}

void Internal_LogInfo(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_INFO_LOC(file.c_str(), line, func.c_str(), message);
}

void Internal_LogWarning(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_WARNING_LOC(file.c_str(), line, func.c_str(), message);
}

void Internal_LogError(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_ERROR_LOC(file.c_str(), line, func.c_str(), message);
}

} // namespace

auto script_system::bind_internal_calls(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    mono::add_internal_call("Ace.Core.Log::Internal_LogTrace", internal_call(Internal_LogTrace));
    mono::add_internal_call("Ace.Core.Log::Internal_LogInfo", internal_call(Internal_LogInfo));
    mono::add_internal_call("Ace.Core.Log::Internal_LogWarning", internal_call(Internal_LogWarning));
    mono::add_internal_call("Ace.Core.Log::Internal_LogError", internal_call(Internal_LogError));

    mono::add_internal_call("Ace.Core.Scene::Internal_CreateScene", internal_call(Internal_CreateScene));
    mono::add_internal_call("Ace.Core.Scene::Internal_DestroyScene", internal_call(Internal_DestroyScene));
    mono::add_internal_call("Ace.Core.Scene::Internal_CreateEntity", internal_call(Internal_CreateEntity));
    mono::add_internal_call("Ace.Core.Scene::Internal_DestroyEntity", internal_call(Internal_DestroyEntity));
    mono::add_internal_call("Ace.Core.Scene::Internal_IsEntityValid", internal_call(Internal_IsEntityValid));

    mono::add_internal_call("Ace.Core.Entity::Internal_AddComponent", internal_call(Internal_AddComponent));
    mono::add_internal_call("Ace.Core.Entity::Internal_GetComponent", internal_call(Internal_GetComponent));
    mono::add_internal_call("Ace.Core.Entity::Internal_RemoveComponent", internal_call(Internal_RemoveComponent));

    // mono::managed_interface::init(assembly);

    return true;
}

} // namespace ace
