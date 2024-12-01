#pragma once
#include "tween_core.h"
#include "tween_inspector.h"
#include "tween_updater.h"
#include "tween_ease.h"

namespace tweeny
{

template<typename T>
tween_action
tween_from_to(T& object,
              const std::decay_t<T>& begin,
              const std::decay_t<T>& end,
			  const duration_t& duration,
			  const sentinel_t& sentinel,
			  const ease_t& ease_func)
{
    if(sentinel.expired())
    {
        return {};
    }
	auto creator = [&object,
					begin,
					end,
					sentinel,
                    ease_func]()
	{
        auto initialize_func = [begin](T* object, const sentinel_t& sentinel, tween_action& self)
		{
            if(!sentinel.expired())
            {
                *object = begin;
            }
            inspector::update_begin_value(self, begin);
            return begin;
		};

        auto updater_func = [](T* object, const T& next, tween_action& self)
		{
			(*object) = next;
            inspector::update_tween_status(self, *object);
        };

        auto getter_func = [](T* object, tween_action&)
        {
            return *object;
        };

        auto updater = create_tween_updater(&object,
									end,
									sentinel,
									std::move(initialize_func),
                                    std::move(updater_func),
                                    std::move(getter_func),
									ease_func);
        return updater;
	};

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_info(action, __func__, object, end, ease_func);

    return action;
}

template<typename T>
tween_action
tween_from_to(const std::shared_ptr<T>& object,
              const std::decay_t<T>& begin,
              const std::decay_t<T>& end,
			  const duration_t& duration,
			  const ease_t& ease_func)
{
    if(!object)
    {
        return {};
    }
    return tween_from_to(*object.get(), begin, end, duration, object, ease_func);
}

template<typename T>
tween_action
tween_to(T& object,
         const std::decay_t<T>& end,
		 const duration_t& duration,
		 const sentinel_t& sentinel,
		 const ease_t& ease_func)
{
    if(sentinel.expired())
    {
        return {};
    }
	auto creator = [&object,
					end,
					sentinel,
                    ease_func]()
	{
        auto initialize_func = [](T* object, const sentinel_t& sentinel, tween_action& self)
		{
			if(sentinel.expired())
			{
				return T{};
			}

            auto begin = (*object);
            inspector::update_begin_value(self, begin);
            return begin;
		};

        auto updater_func = [](T* object, T next, tween_action& self)
		{
			(*object) = next;
            inspector::update_tween_status(self, *object);
        };

        auto getter_func = [](T* object, tween_action&)
        {
            return *object;
        };
        auto updater = create_tween_updater(&object,
									end,
									sentinel,
									std::move(initialize_func),
                                    std::move(updater_func),
                                    std::move(getter_func),
									ease_func);
        return updater;
	};

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_info(action, __func__, object, end, ease_func);
    return action;
}

template<typename T>
tween_action
tween_to(const std::shared_ptr<T>& object,
         const std::decay_t<T>& end,
		 const duration_t& duration,
		 const ease_t& ease_func)
{
    if(!object)
    {
        return {};
    }
	return tween_to(*object.get(), end, duration, object, ease_func);
}

template<typename T>
tween_action
tween_by(T& object,
         const std::decay_t<T>& amount,
		 const duration_t& duration,
		 const sentinel_t& sentinel,
		 const ease_t& ease_func)
{
    if(sentinel.expired())
    {
        return {};
    }
	auto creator = [&object,
					amount,
					sentinel,
                    ease_func]()
	{
        auto initialize_func = [](T*, const sentinel_t&, tween_action& self)
		{
            auto begin = T{};
            inspector::update_begin_value(self, begin);
            return begin;
		};


        auto updater_func = [prev = T{}](T* object, T next, tween_action& self) mutable
        {
            T step = (next - prev);
            (*object) += step;
			prev = next;

            inspector::update_tween_status(self, *object);
		};

        auto getter_func = [](T* object, tween_action&)
        {
            return *object;
        };

        auto updater = create_tween_updater(&object,
									amount,
									sentinel,
									std::move(initialize_func),
                                    std::move(updater_func),
                                    std::move(getter_func),
									ease_func);
        return updater;
	};

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_info(action, __func__, object, amount, ease_func);
    return action;
}

template<typename T>
tween_action
tween_by(const std::shared_ptr<T>& object,
         const std::decay_t<T>& amount,
         const duration_t& duration,
         const ease_t& ease_func)
{
    if(!object)
    {
        return {};
    }
    return tween_by(*object.get(), amount, duration, object, ease_func);
}
} //end of namespace tweeny
