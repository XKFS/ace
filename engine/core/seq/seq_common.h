#pragma once
#include <chrono>
#include <functional>
#include <hpp/sentinel.hpp>
#include <hpp/type_name.hpp>
#include <memory>
#include <string>
#include <vector>

using namespace std::chrono_literals;

namespace seq
{
using clock_t = std::chrono::steady_clock;
using timepoint_t = clock_t::time_point;

using duraiton_secs_t = std::chrono::duration<float>;
using duration_t = std::chrono::nanoseconds;
using sentinel_t = hpp::sentinel;
using seq_id_t = size_t;
using ease_t = std::function<float(float)>;

template<typename T>
using interpolate_t = std::function<T(const T&, const T&, float, const ease_t&)>;

enum class state_t
{
    running,
    paused,
    finished
};

struct seq_scope_policy
{
    enum class policy_t
    {
        stacked,
        independent
    };

    seq_scope_policy() noexcept = default;

    seq_scope_policy(const char* _scope) noexcept : scope(_scope)
    {
    }

    seq_scope_policy(const std::string& _scope) noexcept : scope(_scope)
    {
    }

    std::string scope;
    policy_t policy = policy_t::stacked;
};

struct seq_action;
struct seq_inspect_info
{
    using ptr = std::shared_ptr<seq_inspect_info>;
    using weak_ptr = std::weak_ptr<seq_inspect_info>;

    std::string file_name;
    std::string function_name;
    uint32_t line_number = 0;
    uint32_t column_offset = 0;

    seq_id_t id = 1;

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
auto as_string(const T& t) -> std::string
{
    return to_string(t);
}

template<typename T>
auto type_as_string(const T&) -> std::string
{
    return hpp::type_name_unqualified_str<T>();
}
} // namespace adl_helper

template<typename T>
auto to_str(const T& t) -> std::string
{
    return adl_helper::as_string(t);
}

template<>
inline auto to_str(const state_t& t) -> std::string
{
    if(t == state_t::running)
    {
        return "running";
    }
    if(t == state_t::paused)
    {
        return "paused";
    }
    return "finished";
}

template<typename T>
auto type_to_str(const T& t) -> std::string
{
    return adl_helper::type_as_string(t);
}

} // end of namespace inspector

} // namespace seq
