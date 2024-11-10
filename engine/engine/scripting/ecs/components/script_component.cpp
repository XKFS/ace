#include "script_component.h"
#include <monopp/mono_property.h>
#include <monopp/mono_property_invoker.h>

#include <engine/engine.h>
#include <engine/events.h>
#include <engine/scripting/ecs/systems/script_system.h>
namespace ace
{

void script_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<script_component>();
    component.set_owner(entity);
}

void script_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
}

void script_component::create()
{
    process_pending_creates();
}
void script_component::start()
{
    process_pending_starts();

    process_pending_deletions();
}

void script_component::destroy()
{
    auto comps = script_components_;

    for(auto& script : comps)
    {
        auto& obj = script.scoped->object;
        remove_script_component(obj);
    }

    process_pending_deletions();
}

void script_component::create(const mono::mono_object& obj)
{
    auto method = mono::make_method_invoker<void()>(obj, "internal_n2m_on_create");
    method(obj);
}
void script_component::start(const mono::mono_object& obj)
{
    auto method = mono::make_method_invoker<void()>(obj, "internal_n2m_on_start");
    method(obj);
}

void script_component::destroy(const mono::mono_object& obj)
{
    auto method = mono::make_method_invoker<void()>(obj, "internal_n2m_on_destroy");
    method(obj);
}

void script_component::set_entity(const mono::mono_object& obj, entt::handle e)
{
    auto method = mono::make_method_invoker<void(uint32_t)>(obj, "internal_n2m_set_entity");
    method(obj, static_cast<uint32_t>(e.entity()));
}

void script_component::process_pending_deletions()
{
    auto& ctx = engine::context();
    auto& ev = ctx.get<events>();

    size_t erased = std::erase_if(script_components_,
                                  [&](const auto& rhs)
                                  {
                                      if(rhs.marked_for_destroy && ev.is_playing)
                                      {
                                          auto& obj = rhs.scoped->object;
                                          destroy(obj);
                                      }

                                      return rhs.marked_for_destroy;
                                  });

    erased += std::erase_if(native_components_,
                            [&](const auto& rhs)
                            {
                                return rhs.marked_for_destroy;
                            });
}

void script_component::process_pending_creates()
{
    while(!script_components_to_create_.empty())
    {
        auto comps = std::move(script_components_to_create_);
        script_components_to_create_.clear();

        for(auto& script : comps)
        {
            auto& obj = script.scoped->object;

            create(obj);
        }
    }
}

void script_component::process_pending_starts()
{
    while(!script_components_to_start_.empty())
    {
        auto comps = std::move(script_components_to_start_);
        script_components_to_start_.clear();

        for(auto& script : comps)
        {
            auto& obj = script.scoped->object;

            start(obj);
        }
    }
}

auto script_component::add_script_component(const mono::mono_type& type) -> script_object
{
    auto obj = type.new_instance();
    return add_script_component(obj);
}

auto script_component::add_script_component(const mono::mono_object& obj) -> script_object
{
    script_object script_obj;
    script_obj.scoped = std::make_shared<mono::mono_scoped_object>(obj);

    return add_script_component(script_obj);
}

auto script_component::add_script_component(const script_object& script_obj) -> script_object
{
    auto& ctx = engine::context();
    auto& sys = ctx.get<script_system>();
    auto& ev = ctx.get<events>();

    script_components_.emplace_back(script_obj);
    script_components_to_create_.emplace_back(script_obj);
    script_components_to_start_.emplace_back(script_obj);

    auto& obj = script_obj.scoped->object;

    set_entity(obj, get_owner());

    if(ev.is_playing && sys.is_create_called())
    {
        process_pending_creates();
    }

    if(ev.is_playing && sys.is_start_called())
    {
        process_pending_starts();
    }

    return script_obj;
}

auto script_component::add_native_component(const mono::mono_type& type) -> script_object
{
    auto& script_obj = native_components_.emplace_back();
    auto obj = type.new_instance();

    script_obj.scoped = std::make_shared<mono::mono_scoped_object>(obj);

    set_entity(obj, get_owner());

    return script_obj;
}

auto script_component::get_script_component(const mono::mono_type& type) -> script_object
{
    auto it = std::find_if(std::begin(script_components_),
                           std::end(script_components_),
                           [&](const auto& rhs)
                           {
                               return rhs.scoped->object.get_type().get_internal_ptr() == type.get_internal_ptr();
                           });

    if(it != std::end(script_components_))
    {
        return *it;
    }

    return {};
}

auto script_component::get_native_component(const mono::mono_type& type) -> script_object
{
    auto it = std::find_if(std::begin(native_components_),
                           std::end(native_components_),
                           [&](const auto& rhs)
                           {
                               return rhs.scoped->object.get_type().get_internal_ptr() == type.get_internal_ptr();
                           });

    if(it != std::end(native_components_))
    {
        return *it;
    }

    return {};
}

auto script_component::remove_script_component(const mono::mono_object& obj) -> bool
{
    auto checker = [&](const auto& rhs)
    {
        return rhs.scoped->object.get_internal_ptr() == obj.get_internal_ptr();
    };
    std::erase_if(script_components_to_create_, checker);

    std::erase_if(script_components_to_start_, checker);

    auto it = std::find_if(std::begin(script_components_),
                           std::end(script_components_),
                           [&](const auto& rhs)
                           {
                               return rhs.scoped->object.get_internal_ptr() == obj.get_internal_ptr();
                           });

    if(it != std::end(script_components_))
    {
        set_entity(obj, {});

        it->marked_for_destroy = true;
        return true;
    }

    return false;
}

auto script_component::remove_native_component(const mono::mono_object& obj) -> bool
{
    auto it = std::find_if(std::begin(native_components_),
                           std::end(native_components_),
                           [&](const auto& rhs)
                           {
                               return rhs.scoped->object.get_internal_ptr() == obj.get_internal_ptr();
                           });

    if(it != std::end(native_components_))
    {
        set_entity(obj, {});

        it->marked_for_destroy = true;
        return true;
    }

    return false;
}

auto script_component::get_script_components() const -> const script_components_t&
{
    return script_components_;
}

} // namespace ace
