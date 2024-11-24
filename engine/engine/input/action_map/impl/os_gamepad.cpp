#include "os_gamepad.hpp"
#include <cmath>

namespace input
{
//  ----------------------------------------------------------------------------
os_gamepad::os_gamepad(int index) : index_(index), name_("Joystick" + std::to_string(index_ + 1))
{
}

//  ----------------------------------------------------------------------------
auto os_gamepad::get_axis_value(uint32_t axis) const -> float
{
    const auto& find = axis_map_.find(axis);

    if(find == axis_map_.end())
    {
        return 0.0f;
    }

    return find->second;
}

//  ----------------------------------------------------------------------------
auto os_gamepad::get_button_state(uint32_t button) const -> button_state
{
    return button_state_map_.get_state(button, button_state::up);
}

//  ----------------------------------------------------------------------------
auto os_gamepad::get_name() const -> const std::string&
{
    return name_;
}

//  ----------------------------------------------------------------------------
auto os_gamepad::is_connected() const -> bool
{
    return false; // glfwJoystickPresent(mIndex) == GLFW_TRUE;
}

//  ----------------------------------------------------------------------------
auto os_gamepad::is_down(uint32_t button) const -> bool
{
    const button_state state = button_state_map_.get_state(button, button_state::up);

    return (state == button_state::down || state == button_state::pressed);
}

//  ----------------------------------------------------------------------------
auto os_gamepad::is_pressed(uint32_t button) const -> bool
{
    return button_state_map_.get_state(button, button_state::up) == button_state::pressed;
}

//  ----------------------------------------------------------------------------
void os_gamepad::update()
{
    // if(!glfwJoystickPresent(mIndex))
    // {
    //     mButtonStateMap.clear();
    //     return;
    // }

    // mButtonStateMap.update();

    // int buttonCount;
    // const unsigned char* buttons = glfwGetJoystickButtons(mIndex, &buttonCount);

    // if(buttons == NULL || buttonCount == 0)
    // {
    //     return;
    // }

    // for(unsigned b = 0; b < buttonCount; ++b)
    // {
    //     const int glfwButtonState = buttons[b];

    //     ButtonState buttonState;
    //     switch(glfwButtonState)
    //     {
    //         case GLFW_PRESS:
    //             buttonState = ButtonState::Pressed;
    //             break;

    //         case GLFW_RELEASE:
    //             buttonState = ButtonState::Released;
    //             break;
    //     }

    //     mButtonStateMap.setState(b, buttonState);
    // }

    // //  Update each axis
    // int axisCount;
    // const float* axes = glfwGetJoystickAxes(mIndex, &axisCount);
    // for(unsigned a = 0; a < axisCount; ++a)
    // {
    //     mAxisMap[a] = axes[a];
    // }
}
} // namespace input
