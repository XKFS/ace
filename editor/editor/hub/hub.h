#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <ospp/event.h>

#include "panels/panel.h"

namespace ace
{

class hub
{
public:
    hub(rtti::context& ctx);
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

private:
    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_frame_render(rtti::context& ctx, delta_t dt);
    void on_frame_ui_render(rtti::context& ctx, delta_t dt);
    void on_play_begin(rtti::context& ctx);
    void on_script_recompile(rtti::context& ctx, const std::string& protocol);
    void on_os_event(rtti::context& ctx, os::event& e);

    void on_start_page_render(rtti::context& ctx);
    void on_opened_project_render(rtti::context& ctx);

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);

    imgui_panels panels_{};

    bool new_project_creator_{};
};
} // namespace ace
