#pragma once

#include "gamepad_axis.hpp"
#include "gamepad_button.hpp"

namespace input
{
class Gamepad360
{
public:
    //  Axes
    static const gamepad_axis LEFT_STICK_X = gamepad_axis::axis1;
    static const gamepad_axis LEFT_STICK_Y = gamepad_axis::axis2;
    static const gamepad_axis LEFT_TRIGGER = gamepad_axis::axis3;
    static const gamepad_axis RIGHT_STICK_X = gamepad_axis::axis4;
    static const gamepad_axis RIGHT_STICK_Y = gamepad_axis::axis5;
    static const gamepad_axis RIGHT_TRIGGER = gamepad_axis::axis6;

    //  Buttons
    static const gamepad_button A_BUTTON = gamepad_button::button1;
    static const gamepad_button B_BUTTON = gamepad_button::button2;
    static const gamepad_button X_BUTTON = gamepad_button::button3;
    static const gamepad_button Y_BUTTON = gamepad_button::button4;
    static const gamepad_button LEFT_BUMPER = gamepad_button::button5;
    static const gamepad_button RIGHT_BUMPER = gamepad_button::button6;
    static const gamepad_button BACK_BUTTON = gamepad_button::button7;
    static const gamepad_button START_BUTTON = gamepad_button::button8;
    static const gamepad_button GUIDE_BUTTON = gamepad_button::button9;
    static const gamepad_button LEFT_THUMB = gamepad_button::button10;
    static const gamepad_button RIGHT_THUMB = gamepad_button::button11;
    static const gamepad_button DPAD_UP = gamepad_button::button12;
    static const gamepad_button DPAD_RIGHT = gamepad_button::button13;
    static const gamepad_button DPAD_DOWN = gamepad_button::button14;
    static const gamepad_button DPAD_LEFT = gamepad_button::button15;
};
} // namespace InputLib
