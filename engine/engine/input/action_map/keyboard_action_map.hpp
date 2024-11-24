#pragma once

#include "action_id.hpp"
#include "key.hpp"
#include "keyboard.hpp"
#include <map>
#include <vector>

namespace input
{
class keyboard_action_map
{
    struct key_entry
    {
        key_code key{};
        //  Analog value used when key is down
        float analog_value{};
    };

    std::map<action_id_t, std::vector<key_entry>> entries_by_action_id_;

public:
    auto get_analog_value(const action_id_t& action, const keyboard& device) const -> float;
    auto get_digital_value(const action_id_t& action, const keyboard& device) const -> bool;
    auto is_pressed(const action_id_t& action, const keyboard& device) const -> bool;
    auto is_released(const action_id_t& action, const keyboard& device) const -> bool;
    auto is_down(const action_id_t& action, const keyboard& device) const -> bool;

    void map(const action_id_t& action, key_code key, float analog_value = 1.0f);
};
} // namespace input
