#pragma once
#include "tween_common.h"
#include "tween_private.h"
#include "tween_ease.h"
#include <vector>

namespace tweeny
{

template<typename T>
tween_action
tween_from_to(T& object,
              const std::decay_t<T>& begin,
              const std::decay_t<T>& end,
			  const duration_t& duration,
			  const sentinel_t& sentinel,
			  const ease_t& ease_func = ease::linear);

template<typename T>
tween_action
tween_from_to(const std::shared_ptr<T>& object,
              const std::decay_t<T>& begin,
              const std::decay_t<T>& end,
			  const duration_t& duration,
			  const ease_t& ease_func = ease::linear);

template<typename T>
tween_action
tween_to(T& object,
         const std::decay_t<T>& end,
		 const duration_t& duration,
		 const sentinel_t& sentinel,
		 const ease_t& ease_func = ease::linear);

template<typename T>
tween_action
tween_to(const std::shared_ptr<T>& object,
         const std::decay_t<T>& end,
		 const duration_t& duration,
		 const ease_t& ease_func = ease::linear);

template<typename T>
tween_action
tween_by(T& object,
         const std::decay_t<T>& amount,
         const duration_t& duration,
         const sentinel_t& sentinel,
		 const ease_t& ease_func = ease::linear);

template<typename T>
tween_action
tween_by(const std::shared_ptr<T>& object,
         const std::decay_t<T>& amount,
         const duration_t& duration,
         const ease_t& ease_func = ease::linear);


tween_action sequence(const std::vector<tween_action>& tweenies, const sentinel_t& sentinel = hpp::eternal_sentinel());
tween_action sequence_precise(const std::vector<tween_action>& tweenies, const sentinel_t& sentinel = hpp::eternal_sentinel());

template<typename... Args>
tween_action sequence(const tween_action& t1, const tween_action& t2, Args&&... tweenies)
{
    return sequence(std::vector<tween_action>{t1, t2, std::forward<Args>(tweenies)...});
}

template<typename... Args>
tween_action sequence_precise(const tween_action& t1, const tween_action& t2, Args&&... tweenies)
{
    return sequence_precise(std::vector<tween_action>{t1, t2, std::forward<Args>(tweenies)...});
}

tween_action together(const std::vector<tween_action>& tweenies, const sentinel_t& sentinel = hpp::eternal_sentinel());

template<typename... Args>
tween_action together(const tween_action& t1, const tween_action& t2, Args&&... tweenies)
{
    return together(std::vector<tween_action>{t1, t2, std::forward<Args>(tweenies)...});
}

tween_action delay(const duration_t& duration, const sentinel_t& sentinel = hpp::eternal_sentinel());

tween_action repeat(const tween_action& tween, size_t times = 0);
tween_action repeat(const tween_action& tween, const sentinel_t& sentinel, size_t times = 0);

tween_action repeat_precise(const tween_action& tween, size_t times = 0);
tween_action repeat_precise(const tween_action& tween, const sentinel_t& sentinel, size_t times = 0);


} //end of namespace tweeny

#include "tween_core.hpp"
