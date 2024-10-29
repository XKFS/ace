#include "script_component.h"

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

void script_component::set_scripts(const script_collection_t& scripts)
{
    scripts_ = scripts;
}

auto script_component::get_scripts() const -> const script_collection_t&
{
    return scripts_;
}

} // namespace ace
