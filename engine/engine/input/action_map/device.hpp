#pragma once

#include "device_type.hpp"
#include <string>

namespace input
{
class input_device
{
    device_type type_;

public:
    input_device(const device_type type) : type_(type)
    {
    }

    virtual ~input_device()
    {
    }

    auto get_device_type() const -> device_type
    {
        return type_;
    }

    virtual auto get_name() const -> const std::string& = 0;
};
} // namespace input
