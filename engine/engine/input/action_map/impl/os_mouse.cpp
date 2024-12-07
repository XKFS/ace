#include "os_mouse.hpp"
#include "../mouse_action_map.hpp"
namespace input
{
//  ----------------------------------------------------------------------------
auto os_mouse::get_button_state(uint32_t button) const -> button_state
{
    return button_state_map_.get_state(button, button_state::up);
}

//  ----------------------------------------------------------------------------
auto os_mouse::get_button_state_map() -> button_state_map&
{
    return button_state_map_;
}

auto os_mouse::get_axis_value(uint32_t axis) const -> float
{
    const auto& find = axis_map_.find(axis);

    if(find == axis_map_.end())
    {
        return 0.0f;
    }

    return find->second;
}

//  ----------------------------------------------------------------------------
auto os_mouse::get_left_button_state() const -> button_state
{
    return get_button_state(mouse::LEFT_BUTTON);
}

//  ----------------------------------------------------------------------------
auto os_mouse::get_middle_button_state() const -> button_state
{
    return get_button_state(mouse::MIDDLE_BUTTON);
}

//  ----------------------------------------------------------------------------
auto os_mouse::get_name() const -> const std::string&
{
    static const std::string name = "Mouse";
    return name;
}

//  ----------------------------------------------------------------------------
auto os_mouse::get_position() const -> coord
{
    return position_;
}

//  ----------------------------------------------------------------------------
auto os_mouse::get_right_button_state() const -> button_state
{
    return get_button_state(mouse::RIGHT_BUTTON);
}

//  ----------------------------------------------------------------------------
auto os_mouse::get_scroll() const -> float
{
    return scroll_;
}

//  ----------------------------------------------------------------------------
auto os_mouse::is_down(uint32_t button) const -> bool
{
    const button_state state = get_button_state(button);
    return (state == button_state::down || state == button_state::pressed);
}

//  ----------------------------------------------------------------------------
auto os_mouse::is_pressed(uint32_t button) const -> bool
{
    return get_button_state(button) == button_state::pressed;
}

auto os_mouse::is_released(uint32_t button) const -> bool
{
    return get_button_state(button) == button_state::released;
}

//  ----------------------------------------------------------------------------
void os_mouse::set_position(coord  pos)
{
    axis_map_[int(mouse_axis::x)] = float(pos.x - position_.x);
    axis_map_[int(mouse_axis::y)] = float(position_.y - pos.y);

    position_ = pos;
}

//  ----------------------------------------------------------------------------
void os_mouse::set_scroll(float scroll)
{
    axis_map_[int(mouse_axis::scroll)] = scroll - scroll_;

    scroll_ = scroll;
}

//  ----------------------------------------------------------------------------
void os_mouse::update()
{
    button_state_map_.update();

    scroll_ = 0;

    for(auto& kvp : axis_map_)
    {
        kvp.second = 0.0f;
    }
}
} // namespace input
