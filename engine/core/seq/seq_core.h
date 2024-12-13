#pragma once
#include "seq_common.h"
#include "seq_ease.h"
#include "seq_private.h"
#include <vector>

namespace seq
{

template<typename T>
auto change_from_to(T& object,
                    const std::decay_t<T>& begin,
                    const std::decay_t<T>& end,
                    const duration_t& duration,
                    const sentinel_t& sentinel,
                    const ease_t& ease_func = ease::linear) -> seq_action;

template<typename T>
auto change_from_to(const std::shared_ptr<T>& object,
                    const std::decay_t<T>& begin,
                    const std::decay_t<T>& end,
                    const duration_t& duration,
                    const ease_t& ease_func = ease::linear) -> seq_action;

template<typename T>
auto change_to(T& object,
               const std::decay_t<T>& end,
               const duration_t& duration,
               const sentinel_t& sentinel,
               const ease_t& ease_func = ease::linear) -> seq_action;

template<typename T>
auto change_to(const std::shared_ptr<T>& object,
               const std::decay_t<T>& end,
               const duration_t& duration,
               const ease_t& ease_func = ease::linear) -> seq_action;

template<typename T>
auto change_by(T& object,
               const std::decay_t<T>& amount,
               const duration_t& duration,
               const sentinel_t& sentinel,
               const ease_t& ease_func = ease::linear) -> seq_action;

template<typename T>
auto change_by(const std::shared_ptr<T>& object,
               const std::decay_t<T>& amount,
               const duration_t& duration,
               const ease_t& ease_func = ease::linear) -> seq_action;

auto sequence(const std::vector<seq_action>& actions, const sentinel_t& sentinel = hpp::eternal_sentinel())
    -> seq_action;
auto sequence_precise(const std::vector<seq_action>& actions, const sentinel_t& sentinel = hpp::eternal_sentinel())
    -> seq_action;

template<typename... Args>
auto sequence(const seq_action& t1, const seq_action& t2, Args&&... actions) -> seq_action
{
    return sequence(std::vector<seq_action>{t1, t2, std::forward<Args>(actions)...});
}

template<typename... Args>
auto sequence_precise(const seq_action& t1, const seq_action& t2, Args&&... actions) -> seq_action
{
    return sequence_precise(std::vector<seq_action>{t1, t2, std::forward<Args>(actions)...});
}

auto together(const std::vector<seq_action>& actions, const sentinel_t& sentinel = hpp::eternal_sentinel())
    -> seq_action;

template<typename... Args>
auto together(const seq_action& t1, const seq_action& t2, Args&&... actions) -> seq_action
{
    return together(std::vector<seq_action>{t1, t2, std::forward<Args>(actions)...});
}

auto delay(const duration_t& duration, const sentinel_t& sentinel = hpp::eternal_sentinel()) -> seq_action;

auto repeat(const seq_action& action, size_t times = 0) -> seq_action;
auto repeat(const seq_action& action, const sentinel_t& sentinel, size_t times = 0) -> seq_action;

auto repeat_precise(const seq_action& action, size_t times = 0) -> seq_action;
auto repeat_precise(const seq_action& action, const sentinel_t& sentinel, size_t times = 0) -> seq_action;

} // namespace seq

#include "seq_core.hpp"
