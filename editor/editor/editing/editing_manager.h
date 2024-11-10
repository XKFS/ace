#pragma once
#include "editor_actions.h"

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <math/math.h>
#include <rttr/variant.h>

#include <engine/ecs/components/transform_component.h>
#include <engine/rendering/ecs/components/camera_component.h>

#include <editor/imgui/integration/imgui.h>

namespace ace
{
struct editing_manager
{
    struct selection
    {
        rttr::variant object;
    };

    struct focused
    {
        rttr::variant object;
        int frames{};

        fs::path focus_path{};
    };

    struct snap
    {
        ///
        math::vec3 translation_snap = {1.0f, 1.0f, 1.0f};
        ///
        float rotation_degree_snap = 15.0f;
        ///
        float scale_snap = 0.1f;
    };

    struct grid
    {
        float opacity = 1.0f;
    };

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void on_play_before_begin(rtti::context& ctx);
    void on_play_after_end(rtti::context& ctx);
    void on_frame_update(rtti::context& ctx, delta_t);

    //-----------------------------------------------------------------------------
    //  Name : select ()
    /// <summary>
    /// Selects an object. Can be anything.
    /// </summary>
    //-----------------------------------------------------------------------------
    void select(rttr::variant object);
    void focus(rttr::variant object);
    void focus_path(const fs::path& object);

    //-----------------------------------------------------------------------------
    //  Name : unselect ()
    /// <summary>
    /// Clears the selection data.
    /// </summary>
    //-----------------------------------------------------------------------------
    void unselect();
    void unfocus();

    //-----------------------------------------------------------------------------
    //  Name : try_unselect ()
    /// <summary>
    /// Clears the selection data if it maches the type.
    /// </summary>
    //-----------------------------------------------------------------------------
    template<typename T>
    void try_unselect()
    {
        if(selection_data.object.is_type<T>())
        {
            unselect();
        }
    }

    template<typename T>
    void try_unfocus()
    {
        if(focused_data.object.is_type<T>())
        {
            unfocus();
        }
    }

    template<typename T>
    auto is_selected(const T& entry) -> bool
    {
        const auto& selected = selection_data.object;

        if(!selected.is_type<T>())
        {
            return false;
        }

        return selected.get_value<T>() == entry;
    }

    template<typename T>
    auto is_focused(const T& entry) -> bool
    {
        const auto& focused = focused_data.object;

        if(!focused.is_type<T>())
        {
            return false;
        }

        return focused.get_value<T>() == entry;
    }

    void close_project();

    /// enable editor grid
    bool show_grid = true;
    /// enable wireframe selection
    bool wireframe_selection = true;
    /// current manipulation gizmo operation.
    ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
    /// current manipulation gizmo space.
    ImGuizmo::MODE mode = ImGuizmo::LOCAL;
    /// selection data containing selected object
    selection selection_data;

    focused focused_data;

    /// snap data containging various snap options
    snap snap_data;
    grid grid_data;

private:
    void on_script_recompile(rtti::context& ctx, const std::string& protocol);
    void save_checkpoint(rtti::context& ctx);
    void load_checkpoint(rtti::context& ctx);

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);

    std::stringstream scene_cache_;
    asset_handle<scene_prefab> scene_cache_source_;

};
} // namespace ace
