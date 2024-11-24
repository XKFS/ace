#include "input.h"

#include <logging/logging.h>

namespace ace
{
auto input_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    // default mapping
    mapper.map("Mouse X", input::mouse_axis::x);
    mapper.map("Mouse Y", input::mouse_axis::y);
    mapper.map("Mouse ScrollWheel", input::mouse_axis::scroll);

    mapper.map("Horizontal", input::key_code::a, -1.0f);
    mapper.map("Horizontal", input::key_code::d, 1.0f);
    mapper.map("Horizontal", input::key_code::left, -1.0f);
    mapper.map("Horizontal", input::key_code::r, 1.0f);

    mapper.map("Vertical", input::key_code::w, 1.0f);
    mapper.map("Vertical", input::key_code::s, -1.0f);
    mapper.map("Vertical", input::key_code::up, 1.0f);
    mapper.map("Vertical", input::key_code::down, -1.0f);

    mapper.map("Jump", input::key_code::space, 1.0f);
    mapper.map("Submit", input::key_code::enter, 1.0f);
    mapper.map("Cancel", input::key_code::escape, 1.0f);

    return true;
}

auto input_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto input_system::get_analog_value(const input::action_id_t& action) const -> float
{
    float analog = 0.0f;

    for(const auto& device : manager.get_all_devices())
    {
        analog += mapper.get_analog_value(action, *device);
    }

    return analog;
}

auto input_system::get_digital_value(const input::action_id_t& action) const -> bool
{
    bool digital = false;

    for(const auto& device : manager.get_all_devices())
    {
        digital |= mapper.get_digital_value(action, *device);
    }

    return digital;
}
auto input_system::is_pressed(const input::action_id_t& action) const -> bool
{
    bool pressed = false;

    for(const auto& device : manager.get_all_devices())
    {
        pressed |= mapper.is_pressed(action, *device);
    }

    return pressed;
}

auto input_system::is_released(const input::action_id_t& action) const -> bool
{
    bool released = false;

    for(const auto& device : manager.get_all_devices())
    {
        released |= mapper.is_released(action, *device);
    }

    return released;
}

auto input_system::is_down(const input::action_id_t& action) const -> bool
{
    bool down = false;

    for(const auto& device : manager.get_all_devices())
    {
        down |= mapper.is_down(action, *device);
    }

    return down;
}

} // namespace ace
