#pragma once
#include <engine/engine_export.h>

#include <engine/assets/asset_handle.h>
#include <engine/ecs/components/basic_component.h>
#include <engine/physics/ecs/components/physics_component.h>

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
    static void on_create_component(entt::registry& r, entt::entity e);

    /**
     * @brief Called when the component is destroyed.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_destroy_component(entt::registry& r, entt::entity e);

    void process_pending_deletions();
    void process_pending_creates();
    void process_pending_starts();

    auto add_script_component(const mono::mono_type& type) -> script_object;
    auto add_script_component(const mono::mono_object& obj) -> script_object;
    auto add_script_component(const script_object& obj) -> script_object;

    auto add_native_component(const mono::mono_type& type) -> script_object;

    auto remove_script_component(const mono::mono_object& obj) -> bool;
    auto remove_script_component(const mono::mono_type& type) -> bool;

    auto remove_native_component(const mono::mono_object& obj) -> bool;
    auto remove_native_component(const mono::mono_type& type) -> bool;

    auto get_script_components(const mono::mono_type& type) -> std::vector<mono::mono_object>;
    auto get_script_component(const mono::mono_type& type) -> script_object;
    auto get_native_component(const mono::mono_type& type) -> script_object;

    auto get_script_components() const -> const script_components_t&;
    auto has_script_components() const -> bool;

    void create();
    void start();
    void destroy();



    void on_sensor_enter(entt::handle other);
    void on_sensor_exit(entt::handle other);

    void on_collision_enter(entt::handle other, const std::vector<manifold_point>& manifolds, bool use_b);
    void on_collision_exit(entt::handle other, const std::vector<manifold_point>& manifolds, bool use_b);

private:


    void create(const mono::mono_object& obj);
    void start(const mono::mono_object& obj);
    void destroy(const mono::mono_object& obj);
    void set_entity(const mono::mono_object& obj, entt::handle e);
    void on_sensor_enter(const mono::mono_object& obj, entt::handle other);
    void on_sensor_exit(const mono::mono_object& obj, entt::handle other);

    void on_collision_enter(const mono::mono_object& obj, entt::handle other, const std::vector<manifold_point>& manifolds, bool use_b);
    void on_collision_exit(const mono::mono_object& obj, entt::handle other, const std::vector<manifold_point>& manifolds, bool use_b);

    script_components_t script_components_;
    script_components_t script_components_to_create_;
    script_components_t script_components_to_start_;

    script_components_t native_components_;
};

} // namespace ace
