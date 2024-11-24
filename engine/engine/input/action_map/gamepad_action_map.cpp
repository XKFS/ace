#include "gamepad_action_map.hpp"

namespace input
{
//  ----------------------------------------------------------------------------
namespace
{
auto get_axis_value(const float value,
                    const axis_range range,
                    const float min_analog_value,
                    const float max_analog_value) -> float
{
    float v{};

    switch(range)
    {
        case axis_range::full:
            v = value;
            break;
        case axis_range::positive:
            v = value > 0.0f ? value : 0.0f;
            break;
        case axis_range::negative:
            v = value < 0.0f ? value : 0.0f;
            break;
        default:
            throw std::runtime_error("Case not implemented.");
    }

    const float inStart = -1.0f;
    const float inEnd = 1.0f;

    return (v - inStart) / (inEnd - inStart) * (max_analog_value - min_analog_value) + min_analog_value;
}
} // namespace
//  ----------------------------------------------------------------------------
auto gamepad_action_map::get_analog_value(const action_id_t& action, const gamepad& device) const -> float
{
    const auto& find = entries_by_action_id_.find(action);

    if(find == entries_by_action_id_.end())
    {
        return 0.0f;
    }

    const auto& entries = find->second;

    for(const auto& entry : entries)
    {
        switch(entry.type)
        {
            case input_type::axis:
            {
                const float value = get_axis_value(device.get_axis_value(entry.value),
                                                   entry.range,
                                                   entry.min_analog_value,
                                                   entry.max_nalog_value);

                if(value != 0.0f)
                {
                    return value;
                }
            }
            break;

            case input_type::button:
            {
                const button_state state = device.get_button_state(entry.value);
                const float value = button_state_to_analog_value(state);
                if(value != 0.0f)
                {
                    return value;
                }
            }
            break;
            default:
                break;
        }
    }

    return 0.0f;
}

//  ----------------------------------------------------------------------------
auto gamepad_action_map::get_digital_value(const action_id_t& action, const gamepad& device) const -> bool
{
    const auto& find = entries_by_action_id_.find(action);

    if(find == entries_by_action_id_.end())
    {
        return false;
    }

    const auto& entries = find->second;

    for(const auto& entry : entries)
    {
        switch(entry.type)
        {
            case input_type::axis:
            {
                const float value = get_axis_value(device.get_axis_value(entry.value),
                                                   entry.range,
                                                   entry.min_analog_value,
                                                   entry.max_nalog_value);

                if(value != 0.0f)
                {
                    return true;
                }
            }
            break;

            case input_type::button:
            {
                const button_state state = device.get_button_state(entry.value);
                const bool value = button_state_to_digital_value(state);
                if(value)
                {
                    return true;
                }
            }
            break;
            default:
                break;
        }
    }

    return false;
}

//  ----------------------------------------------------------------------------
auto gamepad_action_map::is_pressed(const action_id_t& action, const gamepad& device) const -> bool
{
    const auto& find = entries_by_action_id_.find(action);

    if(find == entries_by_action_id_.end())
    {
        return false;
    }

    const auto& entries = find->second;

    for(const auto& entry : entries)
    {
        switch(entry.type)
        {
            case input_type::axis:
            {
            }
            break;

            case input_type::button:
            {
                const button_state state = device.get_button_state(entry.value);
                return state == button_state::pressed;
            }
            break;

            default:
                break;
        }
    }

    return false;
}

auto gamepad_action_map::is_released(const action_id_t& action, const gamepad& device) const -> bool
{
    const auto& find = entries_by_action_id_.find(action);

    if(find == entries_by_action_id_.end())
    {
        return false;
    }

    const auto& entries = find->second;

    for(const auto& entry : entries)
    {
        switch(entry.type)
        {
            case input_type::axis:
            {
            }
            break;

            case input_type::button:
            {
                const button_state state = device.get_button_state(entry.value);
                return state == button_state::released;
            }
            break;

            default:
                break;
        }
    }

    return false;
}

auto gamepad_action_map::is_down(const action_id_t& action, const gamepad& device) const -> bool
{
    const auto& find = entries_by_action_id_.find(action);

    if(find == entries_by_action_id_.end())
    {
        return false;
    }

    const auto& entries = find->second;

    for(const auto& entry : entries)
    {
        switch(entry.type)
        {
            case input_type::axis:
            {
            }
            break;

            case input_type::button:
            {
                const button_state state = device.get_button_state(entry.value);
                return state == button_state::down;
            }
            break;

            default:
                break;
        }
    }

    return false;
}

//  ----------------------------------------------------------------------------
void gamepad_action_map::map(const action_id_t& action,
                             gamepad_axis axis,
                             axis_range range,
                             float min_analog_value,
                             float max_analog_value)
{
    gamepad_entry entry = {};
    entry.type = input_type::axis;
    entry.range = range;
    entry.min_analog_value = std::min(min_analog_value, max_analog_value);
    entry.max_nalog_value = std::max(min_analog_value, max_analog_value);
    entry.value = static_cast<uint32_t>(axis);

    entries_by_action_id_[action].push_back(entry);
}

//  ----------------------------------------------------------------------------
void gamepad_action_map::map(const action_id_t& action, gamepad_button button)
{
    gamepad_entry entry = {};
    entry.type = input_type::button;
    entry.value = static_cast<uint32_t>(button);

    entries_by_action_id_[action].push_back(entry);
}
} // namespace input
