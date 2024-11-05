#pragma once
#include <engine/engine_export.h>

#include <engine/assets/asset_handle.h>
#include <engine/ecs/components/basic_component.h>
#include <engine/scripting/script.h>
#include <monort/monort.h>

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
    using scoped_object_ptr = std::shared_ptr<mono::mono_scoped_object>;
    struct script_object
    {
        scoped_object_ptr scoped;
        bool marked_for_destroy{};
    };
    using script_components_t = std::vector<script_object>;
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

    void process_pending_deletions();

    auto add_script_component(const mono::mono_type& type) -> script_object;
    auto add_native_component(const mono::mono_type& type) -> script_object;

    auto remove_script_component(const mono::mono_object& obj) -> bool;
    auto remove_native_component(const mono::mono_object& obj) -> bool;

    auto get_script_component(const mono::mono_type& type) -> script_object;
    auto get_native_component(const mono::mono_type& type) -> script_object;

    auto get_script_components() const -> const script_components_t&;

private:
    script_components_t script_components_;
    script_components_t native_components_;
};

} // namespace ace
