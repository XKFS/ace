#include "tween_core.h"
#include "tween_inspector.h"

namespace tweeny
{

tween_action sequence_impl(const std::vector<tween_action>& tweenies, const sentinel_t& sentinel, bool precise)
{
    if(tweenies.empty())
    {
        return tween_action();
    }

    duration_t duration = 0ms;
    for(auto& tween : tweenies)
    {
        duration += tween_private::get_duration(tween);
    }

    auto updater = [tweenies = tweenies
                    , sentinel = sentinel
                    , current_tween_idx = size_t(0)
                    , prev_overflow = duration_t::zero()
                    , prev_elapsed = duration_t::zero()
                    , start_required = true
                    , finished = false
                    , precise]
        (duration_t delta, tween_action& self) mutable
    {
        if(finished)
        {
            return state_t::finished;
        }

        if(sentinel.expired())
        {
            finished = true;
            return state_t::finished;
        }

        if(start_required)
        {
            tween_private::start(tweenies.at(current_tween_idx));
            prev_elapsed = duration_t::zero();
            start_required = false;
        }

        if(tween_private::is_stop_when_finished_requested(self))
        {
            for(auto& tween : tweenies)
            {
                tween_private::stop_when_finished(tween);
            }
        }

        auto& current_tween = tweenies.at(current_tween_idx);

        if(precise)
        {
            delta += prev_overflow;
        }

        auto state = tween_private::update(current_tween, delta);


        auto elapsed = tween_private::get_elapsed(current_tween);
        auto elapsed_diff = elapsed - prev_elapsed;
        prev_elapsed = elapsed;


        tween_private::update_elapsed(self, elapsed_diff);
        inspector::update_tween_status(self, current_tween_idx);
        self.on_step.emit();

        prev_overflow = duration_t::zero();
        if(state == state_t::finished)
        {
            prev_overflow = tween_private::get_overflow(current_tween);
            current_tween_idx++;
            if(current_tween_idx == tweenies.size())
            {
                tween_private::update_elapsed(self, prev_overflow);

                finished = true;
                return state_t::finished;
            }
            start_required = true;
        }
        return state_t::running;
    };

    auto creator = [updater = std::move(updater)]()
    {
        return updater;
    };

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_sequence_info(action, tweenies);
    return action;
}

tween_action sequence(const std::vector<tween_action>& tweenies, const sentinel_t& sentinel)
{
    return sequence_impl(tweenies, sentinel, false);
}

tween_action sequence_precise(const std::vector<tween_action>& tweenies, const sentinel_t& sentinel)
{
    return sequence_impl(tweenies, sentinel, true);
}

tween_action together(const std::vector<tween_action>& tweenies, const sentinel_t& sentinel)
{
    if(tweenies.empty())
    {
        return tween_action();
    }

    duration_t duration = 0ms;
    for(const auto& tween : tweenies)
    {
        duration = std::max(tween_private::get_duration(tween), duration);
    }

    auto updater = [tweenies = tweenies
                    , sentinel = sentinel
                    , start_required = true
                    , finished = false](duration_t delta, tween_action& self) mutable
    {
        if(finished)
        {
            return state_t::finished;
        }

        if(sentinel.expired())
        {
            finished = true;
            return state_t::finished;
        }

        if(start_required)
        {
            for(auto& tween : tweenies)
            {
                tween_private::start(tween);
            }
            start_required = false;
        }

        if(tween_private::is_stop_when_finished_requested(self))
        {
            for(auto& tween : tweenies)
            {
                tween_private::stop_when_finished(tween);
            }
        }

        finished = true;
        for(auto& tween : tweenies)
        {
            const auto state = tween_private::update(tween, delta);
            finished &= (state == state_t::finished);
        }

        tween_private::update_elapsed(self, delta);
        inspector::update_tween_status(self);
        self.on_step.emit();

        if(finished)
        {
            return state_t::finished;
        }
        return state_t::running;
    };

    auto creator = [updater = std::move(updater)]()
    {
        return updater;
    };

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_together_info(action, tweenies);
    return action;
}

tween_action delay(const duration_t& duration, const sentinel_t& sentinel)
{
    auto updater = [sentinel, finished = false](duration_t delta, tween_action& self) mutable
    {
        if(finished)
        {
            return state_t::finished;
        }

        if(sentinel.expired())
        {
            finished = true;
            return state_t::finished;
        }

        tween_private::update_elapsed(self, delta);
        inspector::update_tween_status(self);

        self.on_step.emit();

        if(tween_private::get_elapsed(self) == tween_private::get_duration(self))
        {
            finished = true;
            return state_t::finished;
        }

        return state_t::running;
    };

    auto creator = [updater = std::move(updater)]()
    {
        return updater;
    };

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_delay_info(action);
    return action;
}


tween_action repeat_impl(const tween_action& tween, size_t times, bool precise, const sentinel_t& sentinel = hpp::eternal_sentinel())
{
    auto updater = [tween = tween,
                    times = times,
                    sentinel = sentinel,
                    elapsed = size_t(0),
                    finished = false,
                    prev_overflow = duration_t::zero(),
                    precise]
        (duration_t delta, tween_action& self) mutable
    {
        if(finished)
        {
            return state_t::finished;
        }

        if(sentinel.expired())
        {
            finished = true;
            return state_t::finished;
        }

        if(elapsed > 0)
        {
            if(tween_private::is_stop_when_finished_requested(self))
            {
                tween_private::stop_when_finished(tween);
            }
        }

        if(precise)
        {
            delta += prev_overflow;
        }

        auto state = tween_private::update(tween, delta);
        if(times > 0)
        {
            inspector::update_tween_status(self, elapsed);
        }

        self.on_step.emit();

        prev_overflow = duration_t::zero();
        if(state == state_t::finished)
        {
            prev_overflow = tween_private::get_overflow(tween);
            if(elapsed > 0)
            {
                if(tween_private::is_stop_when_finished_requested(self))
                {
                    finished = true;
                    return state_t::finished;
                }
            }

            if(times == 0)
            {
                tween_private::start(tween);
                elapsed++;
            }
            else
            {
                if(elapsed >= times)
                {
                    finished = true;
                    return state_t::finished;
                }

                tween_private::start(tween);

                elapsed++;
            }
        }
        return state_t::running;
    };

    auto creator = [updater = std::move(updater)]() mutable
    {
        return updater;
    };

    duration_t duration = 0ms;
    if(times > 0)
    {
        duration = times * tween_private::get_duration(tween);
    }

    auto action = tween_action(std::move(creator), duration, sentinel);
    inspector::add_repeat_info(action, tween, times);
    return action;
}

tween_action repeat(const tween_action& tween, size_t times)
{
    return repeat_impl(tween, times, false);
}

tween_action repeat(const tween_action& tween, const sentinel_t& sentinel, size_t times)
{
    return repeat_impl(tween, times, false, sentinel);
}


tween_action repeat_precise(const tween_action& tween, size_t times)
{
    return repeat_impl(tween, times, true);
}

tween_action repeat_precise(const tween_action& tween, const sentinel_t& sentinel, size_t times)
{
    return repeat_impl(tween, times, true, sentinel);
}

}
