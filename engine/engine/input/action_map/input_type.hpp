#pragma once

namespace input
{
enum class input_type
{
    axis,
    button,
    key,
};

inline auto to_string(input_type type) -> const std::string&
{
    switch(type)
    {
        case input_type::axis:
        {
            static const std::string text = "axis";
            return text;
        }
        case input_type::button:
        {
            static const std::string text = "button";
            return text;
        }
        case input_type::key:
        {
            static const std::string text = "key";
            return text;
        }
        default:
        {
            static const std::string text = "unknown";
            return text;
        }
    }
}
}
