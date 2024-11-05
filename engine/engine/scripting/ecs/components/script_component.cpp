#include "script_component.h"
#include <monopp/mono_property.h>
#include <monopp/mono_property_invoker.h>
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

void script_component::process_pending_deletions()
{
    size_t erased = std::erase_if(script_components_,
                                  [&](const auto& rhs)
                                  {
                                      return rhs.marked_for_destroy;
                                  });

    erased += std::erase_if(native_components_,
                            [&](const auto& rhs)
                            {
                                return rhs.marked_for_destroy;
                            });

    // if(erased > 0)
    // {
    // }
}

auto script_component::add_script_component(const mono::mono_type& type) -> script_object
{
    auto& script_obj = script_components_.emplace_back();
    auto obj = type.new_instance();

    script_obj.scoped = std::make_shared<mono::mono_scoped_object>(obj);

    {
        auto method = mono::make_method_invoker<void(uint32_t, bool)>(obj, "internal_n2m_set_entity");
        method(obj, static_cast<uint32_t>(get_owner().entity()), true);
    }

    {
        auto method = mono::make_method_invoker<void()>(obj, "internal_n2m_on_create");
        method(obj);
    }

    {
        auto method = mono::make_method_invoker<void()>(obj, "internal_n2m_on_start");
        method(obj);
    }

    return script_obj;
}

auto script_component::add_native_component(const mono::mono_type& type) -> script_object
{
    auto& script_obj = native_components_.emplace_back();
    auto obj = type.new_instance();

    script_obj.scoped = std::make_shared<mono::mono_scoped_object>(obj);

    auto method = mono::make_method_invoker<void(uint32_t, bool)>(obj, "internal_n2m_set_entity");
    method(obj, static_cast<uint32_t>(get_owner().entity()), true);

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
    auto it = std::find_if(std::begin(script_components_),
                           std::end(script_components_),
                           [&](const auto& rhs)
                           {
                               return rhs.scoped->object.get_internal_ptr() == obj.get_internal_ptr();
                           });

    if(it != std::end(script_components_))
    {
        auto method = mono::make_method_invoker<void(uint32_t, bool)>(obj, "internal_n2m_set_entity");
        method(obj, static_cast<uint32_t>(entt::handle{}.entity()), false);

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
        auto method = mono::make_method_invoker<void(uint32_t, bool)>(obj, "internal_n2m_set_entity");
        method(obj, static_cast<uint32_t>(entt::handle{}.entity()), false);

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
