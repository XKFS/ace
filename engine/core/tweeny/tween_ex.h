#pragma once
#include "tween_common.h"
#include "tween_updater.h"
#include "tween_core.h"
#include "tween_math.h"
#include <vector>
#include <hpp/utility.hpp>

namespace tweeny
{

template<typename Object, typename T, typename Setter, typename Getter>
tween_action
create_tween_from_to_impl(const std::string& creator_name,
                          Object* object,
                          const T& begin,
                          const T& end,
                          const Setter& setter_func,
                          const Getter& getter_func,
                          const duration_t& duration,
                          const sentinel_t& sentinel,
                          const ease_t& ease_func)
{
    if(!object)
    {
        return {};
    }
    auto creator = [object
                    , begin = begin
                    , end = end
                    , sentinel = sentinel
                    , ease_func = ease_func
                    , setter_func = setter_func
                    , getter_func = getter_func]()
    {
        auto initialize_func = [begin, setter_func]
        (Object* object, const sentinel_t&, tween_action& self)
        {
            hpp::invoke(setter_func, *object, begin);
            inspector::update_begin_value(self, begin);
            return begin;
        };

        auto updater_func = [setter_func, getter_func]
        (Object* object, const T& next, tween_action& self) mutable
        {
            hpp::invoke(setter_func, *object, next);

            //TODO move calc inside
            inspector::update_tween_status(self, hpp::invoke(getter_func, *object));
        };

        auto getter = [getter_func]
            (Object* object, tween_action&) mutable
        {
            return hpp::invoke(getter_func, *object);
        };

        return create_tween_updater(object,
                                    end,
                                    sentinel,
                                    std::move(initialize_func),
                                    std::move(updater_func),
                                    std::move(getter),
                                    ease_func);
    };

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_info(action, creator_name, *object, end, ease_func);
    return action;
}

template<typename Object, typename T, typename Setter, typename Getter>
tween_action
create_tween_to_impl(const std::string& creator_name,
                     Object* object,
                     const T& end,
                     const Setter& setter_func,
                     const Getter& getter_func,
                     const duration_t& duration,
                     const sentinel_t& sentinel,
                     const ease_t& ease_func)
{
    auto creator = [object
                    , end = end
                    , sentinel = sentinel
                    , ease_func = ease_func
                    , setter_func = setter_func
                    , getter_func = getter_func]()
    {
        auto initialize_func = [getter_func]
        (Object* object, const sentinel_t& sentinel, tween_action& self)
        {
            if(sentinel.expired())
            {
                T out{};
                inspector::update_begin_value(self, out);
                return out;
            }

            //TODO move calc inside
            T out = hpp::invoke(getter_func, *object);
            inspector::update_begin_value(self, out);
            return out;
        };

        auto updater_func = [setter_func, getter_func]
        (Object* object, const T& next, tween_action& self) mutable
        {
            hpp::invoke(setter_func, *object, next);

            //TODO move calc inside
            inspector::update_tween_status(self, hpp::invoke(getter_func, *object));
        };

        auto getter = [getter_func]
            (Object* object, tween_action&) mutable
        {
            return hpp::invoke(getter_func, *object);
        };

        return create_tween_updater(object,
                                    end,
                                    sentinel,
                                    std::move(initialize_func),
                                    std::move(updater_func),
                                    std::move(getter),
                                    ease_func);
    };

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_info(action, creator_name, *object, end, ease_func);
    return action;
}

template<typename Object, typename T, typename Setter, typename Getter>
tween_action
create_tween_by_impl(const std::string& creator_name,
                     Object* object,
                     const T& amount,
                     const Setter& setter_func,
                     const Getter& getter_func,
                     const duration_t& duration,
                     const sentinel_t& sentinel,
                     const ease_t& ease_func)
{
    if(!object)
    {
        return {};
    }
    auto creator = [object
                    , amount = amount
                    , sentinel = sentinel
                    , ease_func = ease_func
                    , setter_func = setter_func
                    , getter_func = getter_func]()
    {
        auto initialize_func = []
        (Object*, const sentinel_t&, tween_action& self)
        {
            T out{};
            inspector::update_begin_value(self, out);
            return out;
        };

        auto updater_func = [setter_func, getter_func, prev = T{}]
        (Object* object, const T& next, tween_action& self) mutable
        {
            T current = hpp::invoke(getter_func, *object);

            const auto updated = current + (next - prev);
            hpp::invoke(setter_func, *object, updated);

            //TODO move calc inside
            inspector::update_tween_status(self, hpp::invoke(getter_func, *object));

            prev = next;
        };

        auto getter = [getter_func]
            (Object* object, tween_action&) mutable
        {
            return hpp::invoke(getter_func, *object);
        };

        return create_tween_updater(object,
                                    amount,
                                    sentinel,
                                    std::move(initialize_func),
                                    std::move(updater_func),
                                    std::move(getter),
                                    ease_func);
    };

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_info(action, creator_name, *object, amount, ease_func);
    return action;
}

} //end of namespace tweeny

