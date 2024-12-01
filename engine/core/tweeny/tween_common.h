#pragma once
#include <hpp/sentinel.hpp>
#include <chrono>
#include <memory>
#include <functional>
#include <vector>
#include <string>

using namespace std::chrono_literals;

namespace tweeny
{
using clock_t = std::chrono::steady_clock;
using timepoint_t = clock_t::time_point;

using duration_t = std::chrono::nanoseconds;
using sentinel_t = hpp::sentinel;
using tween_id_t = size_t;
using ease_t = std::function<float(float)>;

template<typename T>
using interpolate_t = std::function<T(const T&, const T&, float, const ease_t&)>;

enum class state_t
{
    running,
    paused,
    finished
};

struct tween_scope_policy
{
    enum class policy_t
    {
        stacked,
        independent
    };

    tween_scope_policy() noexcept = default;

    tween_scope_policy(const char* _scope) noexcept
        : scope(_scope){}

    tween_scope_policy(const std::string& _scope) noexcept
        : scope(_scope) {}

    std::string scope{};
    policy_t policy = policy_t::stacked;
};

struct tween_action;
struct tween_inspect_info
{
    using ptr = std::shared_ptr<tween_inspect_info>;
    using weak_ptr = std::weak_ptr<tween_inspect_info>;

    std::string file_name;
    std::string function_name;
    uint32_t line_number = 0;
    uint32_t column_offset = 0;

    tween_id_t id = 1;

    float speed_multiplier = 1.0f;
    bool stop_when_finished = false;

    std::string state = "finished";
    std::string modified_type;
    std::string updater_type;

    duration_t elapsed{};
    duration_t duration{};
    float progress{};

    std::string current_value;
    std::string begin_value;
    std::string end_value;

    ease_t ease_func;

    std::vector<weak_ptr> children;
};

namespace inspector
{
namespace adl_helper
{
    using std::to_string;

    template<typename T>
    std::string as_string(const T& t)
    {
        return to_string(t);
    }

    template<typename T>
    std::string type_as_string(const T&)
    {
        return "unsupported";
    }
}

template<typename T>
inline std::string to_str(const T& t)
{
    return adl_helper::as_string(t);
}

template<>
inline std::string to_str(const state_t& t)
{
    if(t == state_t::running) return "running";
    if(t == state_t::paused) return "paused";
    return "finished";
}

template<typename T>
inline std::string type_to_str(const T& t)
{
    return adl_helper::type_as_string(t);
}

template<> inline std::string type_to_str(const uint8_t&)  { return "uint8"; }
template<> inline std::string type_to_str(const uint16_t&) { return "uint16"; }
template<> inline std::string type_to_str(const uint32_t&) { return "uint32"; }
template<> inline std::string type_to_str(const uint64_t&) { return "uint64"; }
template<> inline std::string type_to_str(const int8_t&)   { return "int8"; }
template<> inline std::string type_to_str(const int16_t&)  { return "int16"; }
template<> inline std::string type_to_str(const int32_t&)  { return "int32"; }
template<> inline std::string type_to_str(const int64_t&)  { return "int64"; }
template<> inline std::string type_to_str(const float&)    { return "float"; }
template<> inline std::string type_to_str(const double&)   { return "double"; }

} //end of namespace inspector

} //end of namespace tweeny
