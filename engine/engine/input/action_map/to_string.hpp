#pragma once

#include "coord.hpp"
#include "gamepad_axis.hpp"
#include "gamepad_button.hpp"
#include "key_state.hpp"
#include "point.hpp"
#include <stdexcept>
#include <string>

namespace input
{
inline static std::string to_string(const coord& coord)
{
    return "(" + std::to_string(coord.x) + ", " + std::to_string(coord.y) + ")";
}

inline static std::string to_string(const gamepad_axis axis)
{
    switch(axis)
    {
        case gamepad_axis::axis1:
            return "axis 1";
        case gamepad_axis::axis2:
            return "axis 2";
        case gamepad_axis::axis3:
            return "axis 3";
        case gamepad_axis::axis4:
            return "axis 4";
        case gamepad_axis::axis5:
            return "axis 5";
        case gamepad_axis::axis6:
            return "axis 6";
        default:
            throw std::runtime_error("Case not implemented.");
    }
}

inline static std::string to_string(gamepad_button button)
{
    switch(button)
    {
        case gamepad_button::button1:
            return "button 1";
        case gamepad_button::button2:
            return "button 2";
        case gamepad_button::button3:
            return "button 3";
        case gamepad_button::button4:
            return "button 4";
        case gamepad_button::button5:
            return "button 5";
        case gamepad_button::button6:
            return "button 6";
        case gamepad_button::button7:
            return "button 7";
        case gamepad_button::button8:
            return "button 8";
        case gamepad_button::button9:
            return "button 9";
        case gamepad_button::button10:
            return "button 10";
        case gamepad_button::button11:
            return "button 11";
        case gamepad_button::button12:
            return "button 12";
        case gamepad_button::button13:
            return "button 13";
        case gamepad_button::button14:
            return "button 14";
        case gamepad_button::button15:
            return "button 15";
        default:
            throw std::runtime_error("Case not implemented.");
    }
}

inline static std::string to_string(const key_state state)
{
    switch(state)
    {
        case key_state::down:
            return "down";
        case key_state::up:
            return "up";
        case key_state::released:
            return "released";
        case key_state::pressed:
            return "pressed";
    }
}

inline static std::string to_string(const point& p)
{
    return "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
}
} // namespace input
