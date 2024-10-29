#pragma once
#include <engine/engine_export.h>

#include <engine/ecs/components/basic_component.h>
#include <engine/scripting/script.h>
#include <engine/assets/asset_handle.h>
#include <math/math.h>

namespace ace
{

/**
 * @class script_component
 * @brief Class that contains core data for audio listeners.
 * There can only be one instance of it per scene.
 */
class script_component : public component_crtp<script_component, owned_component>
{
public:
    using script_collection_t = std::vector<std::string>;
    /**
     * @brief Called when the component is created.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_create_component(entt::registry& r, const entt::entity e);

    /**
     * @brief Called when the component is destroyed.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    void set_scripts(const script_collection_t& scripts);
    auto get_scripts() const -> const script_collection_t&;


private:

    script_collection_t scripts_;
};

} // namespace ace
