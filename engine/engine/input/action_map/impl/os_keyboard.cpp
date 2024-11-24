#include "os_keyboard.hpp"

namespace input
{
//  ----------------------------------------------------------------------------
auto os_keyboard::get_key_state(key_code key) const -> key_state
{
    return key_state_map_.get_state(key, key_state::up);
}

//  ----------------------------------------------------------------------------
auto os_keyboard::get_name() const -> const std::string&
{
    static const std::string name = "Keyboard";
    return name;
}

//  ----------------------------------------------------------------------------
auto os_keyboard::is_down(key_code key) const -> bool
{
    const key_state keyState = key_state_map_.get_state(key, key_state::up);
    return (keyState == key_state::down || keyState == key_state::pressed);
}

//  ----------------------------------------------------------------------------
auto os_keyboard::is_pressed(key_code key) const -> bool
{
    const key_state keyState = key_state_map_.get_state(key, key_state::up);
    return keyState == key_state::pressed;
}

//  ----------------------------------------------------------------------------
auto os_keyboard::is_released(key_code key) const -> bool
{
    const key_state keyState = key_state_map_.get_state(key, key_state::up);
    return keyState == key_state::released;
}

void os_keyboard::update()
{
    key_state_map_.update();
}

//  ----------------------------------------------------------------------------
auto os_keyboard::get_key_state_map() -> key_state_map&
{
    return key_state_map_;
}
} // namespace input
