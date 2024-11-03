#pragma once
#include <engine/engine_export.h>
#include <engine/scripting/ecs/components/script_component.h>
#include <engine/threading/threader.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <filesystem/filesystem.h>
#include <monort/monort.h>
#include <monopp/mono_method_invoker.h>

namespace ace
{

struct script_system
{
    static void set_needs_recompile(const std::string& protocol);
    static auto get_lib_name(const std::string& protocol) -> std::string;
    static auto get_lib_data_key(const std::string& protocol) -> std::string;
    static auto get_lib_compiled_key(const std::string& protocol) -> std::string;

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void load_core_domain(rtti::context& ctx);
    void unload_core_domain();

    void load_app_domain(rtti::context& ctx);
    void unload_app_domain();

    auto get_all_scriptable_components() const -> const std::vector<mono::mono_type>&;

    /**
     * @brief Called when a physics component is created.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_create_component(entt::registry& r, const entt::entity e);

    /**
     * @brief Called when a physics component is destroyed.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_destroy_component(entt::registry& r, const entt::entity e);

private:
    /**
     * @brief Updates the physics system for each frame.
     * @param ctx The context for the update.
     * @param dt The delta time for the frame.
     */
    void on_frame_update(rtti::context& ctx, delta_t dt);

    /**
     * @brief Called when playback begins.
     * @param ctx The context for the playback.
     */
    void on_play_begin(rtti::context& ctx);

    /**
     * @brief Called when playback ends.
     * @param ctx The context for the playback.
     */
    void on_play_end(rtti::context& ctx);

    /**
     * @brief Called when playback is paused.
     * @param ctx The context for the playback.
     */
    void on_pause(rtti::context& ctx);

    /**
     * @brief Called when playback is resumed.
     * @param ctx The context for the playback.
     */
    void on_resume(rtti::context& ctx);

    /**
     * @brief Skips the next frame update.
     * @param ctx The context for the update.
     */
    void on_skip_next_frame(rtti::context& ctx);

    auto bind_internal_calls(rtti::context& ctx) -> bool;
    auto get_engine_assembly() const -> mono::mono_assembly;
    auto get_app_assembly() const -> mono::mono_assembly;

    void check_for_recompile(rtti::context& ctx, delta_t dt);

    auto create_compilation_job(rtti::context& ctx, const std::string& protocol) -> itc::job_future<bool>;

    ///< Sentinel value to manage shared resources.
    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);

    delta_t time_since_last_check_{};

    std::unique_ptr<mono::mono_domain> domain_;

    struct mono_cache
    {
        mono::mono_type update_manager_type;
        mono::mono_type script_system_type;
        mono::mono_type native_component_type;
        mono::mono_type script_component_type;
    } cache_;

    std::unique_ptr<mono::mono_domain> app_domain_;

    struct mono_app_cache
    {
        std::vector<mono::mono_type> scriptable_component_types;
        std::vector<mono::mono_type> scriptable_system_types;

    } app_cache_;
};
} // namespace ace
