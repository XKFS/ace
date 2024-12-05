#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <editor/editing/editor_actions.h>

namespace ace
{

class imgui_panels;

class project_settings_panel
{
public:
    project_settings_panel(imgui_panels* parent);

    void on_frame_ui_render(rtti::context& ctx, const char* name);

    void show(bool s);

private:
    void draw_ui(rtti::context& ctx);

    imgui_panels* parent_{};
    bool show_request_{};

};
} // namespace ace
