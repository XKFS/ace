#pragma once

#include "../action_map.hpp"
#include "../input_manager.hpp"
#include "os_gamepad.hpp"
#include "os_keyboard.hpp"
#include "os_mouse.hpp"

#include <ospp/event.h>

struct GLFWwindow;

namespace input
{
class os_input_manager : public input_manager
{
    os_keyboard* keyboard_{};
    os_mouse* mouse_{};
    std::map<uint32_t, os_gamepad*> gamepads_{};

    std::vector<std::shared_ptr<input_device>> devices_;
public:
    os_input_manager();

    auto get_all_devices() const -> const std::vector<std::shared_ptr<input_device>>&;
    auto get_gamepad(uint32_t index) const -> const gamepad& override;
    auto get_max_gamepads() const -> uint32_t override;

    auto get_mouse() const -> const mouse& override;

    auto get_keyboard() const -> const keyboard& override;

    void before_events_update() override;
    void after_events_update() override;

    void on_os_event(const os::event& e);
};

} // namespace InputLib
