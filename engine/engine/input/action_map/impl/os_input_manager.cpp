#include "os_input_manager.hpp"
#include "os_key_map.hpp"
#include "../bimap.hpp"
#include <cassert>

namespace input
{

namespace
{
auto get_key_map() -> bimap<key_code, int>&
{
    static bimap<key_code, int> keyMap;
    return keyMap;
}
} // namespace

//  ----------------------------------------------------------------------------
os_input_manager::os_input_manager()
{
    initialize_os_key_map(get_key_map());

    for(uint32_t index = 0; index < get_max_gamepads(); ++index)
    {
        auto gamepad = std::make_shared<os_gamepad>(index);
        devices_.emplace_back(gamepad);
        gamepads_[index] = gamepad.get();
    }

    auto keyboard = std::make_shared<os_keyboard>();
    devices_.emplace_back(keyboard);
    keyboard_ = keyboard.get();

    auto mouse = std::make_shared<os_mouse>();
    devices_.emplace_back(mouse);
    mouse_ = mouse.get();
}

auto os_input_manager::get_all_devices() const -> const std::vector<std::shared_ptr<input_device>>&
{
    return devices_;
}

//  ----------------------------------------------------------------------------
auto os_input_manager::get_gamepad(uint32_t index) const -> const gamepad&
{
    return *gamepads_.at(index);
}

auto os_input_manager::get_max_gamepads() const -> uint32_t
{
    return 16;
}

//  ----------------------------------------------------------------------------
auto os_input_manager::get_mouse() const -> const mouse&
{
    return *mouse_;
}

//  ----------------------------------------------------------------------------
auto os_input_manager::get_keyboard() const -> const keyboard&
{
    return *keyboard_;
}

//  ----------------------------------------------------------------------------
void os_input_manager::before_events_update()
{
    //  Update Pressed and Released states to Down and Up
    keyboard_->update();

    mouse_->update();
}

void os_input_manager::after_events_update()
{
    //  Update gamepads
    for(auto& kvp : gamepads_)
    {
        auto& gamepad = kvp.second;
        gamepad->update();
    }
}

void os_input_manager::on_os_event(const os::event& e)
{
    switch(e.type)
    {
        case os::events::key_down:
        {
            auto& state_map = keyboard_->get_key_state_map();
            key_code key = get_key_map().get_key(e.key.code, key_code::unknown);
            state_map.set_state(key, key_state::pressed);
            break;
        }
        case os::events::key_up:
        {
            auto& state_map = keyboard_->get_key_state_map();
            key_code key = get_key_map().get_key(e.key.code, key_code::unknown);
            state_map.set_state(key, key_state::released);
        }

        case os::events::mouse_button:
        {
            auto& state_map = mouse_->get_button_state_map();

            button_state state{};
            switch(e.button.state_id)
            {
                case os::state::pressed:
                {
                    state = button_state::pressed;
                    break;
                }
                case os::state::released:
                {
                    state = button_state::released;
                    break;
                }
                default:
                    break;
            }

            mouse_button mouse_button {mouse_button::left_button};
            switch(e.button.button)
            {
                case os::mouse::button::left:
                {
                    mouse_button = mouse_button::left_button;
                    break;
                }
                case os::mouse::button::right:
                {
                    mouse_button = mouse_button::right_button;
                    break;
                }
                case os::mouse::button::middle:
                {
                    mouse_button = mouse_button::middle_button;
                    break;
                }
                case os::mouse::button::x1:
                {
                    mouse_button = mouse_button::button4;
                    break;
                }

                case os::mouse::button::x2:
                {
                    mouse_button = mouse_button::button5;
                    break;
                }
                default:
                    break;
            }

            state_map.set_state(static_cast<uint32_t>(mouse_button), state);

            mouse_->set_position({e.button.x, e.button.y});
            break;
        }

        case os::events::mouse_motion:
        {
            mouse_->set_position({e.motion.x, e.motion.y});
            break;
        }

        case os::events::mouse_wheel:
        {
            mouse_->set_scroll(e.wheel.y);
            break;
        }

        default:
            break;

    }


}
} // namespace InputLib
