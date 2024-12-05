#pragma once

namespace input
{
enum class axis_range
{
    full,
    positive,
    negative,
};

inline auto to_string(axis_range range) -> const std::string&
{
    switch(range)
    {
        case axis_range::full:
        {
            static const std::string text = "full";
            return text;
        }
        case axis_range::positive:
        {
            static const std::string text = "positive";
            return text;
        }
        case axis_range::negative:
        {
            static const std::string text = "negative";
            return text;
        }
        default:
        {
            static const std::string text = "unknown";
            return text;
        }
    }
}
} // namespace InputLib
