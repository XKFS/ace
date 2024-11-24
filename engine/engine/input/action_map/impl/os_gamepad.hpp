#pragma once

#include "../bimap.hpp"
#include "../button_state_map.hpp"
#include "../gamepad.hpp"
#include <map>
#include <string>

namespace input
{
class os_gamepad : public gamepad
{
    int index_;
    std::map<unsigned, float> axis_map_;
    button_state_map button_state_map_;
    std::string name_;

public:
    os_gamepad(int index);
    auto get_axis_value(uint32_t axis) const -> float override;
    auto get_button_state(uint32_t button) const -> button_state override;
    auto get_name() const -> const std::string& override;
    auto is_connected() const -> bool override;
    auto is_down(uint32_t button) const -> bool override;
    auto is_pressed(uint32_t button) const -> bool override;
    void update();
};
} // namespace input
