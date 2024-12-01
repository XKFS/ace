#pragma once
#include "tween_action.h"

namespace tweeny
{

struct tween_private
{
    static void start(tween_action& tween)
    {
        tween.start();
    }

    static void stop(tween_action& tween)
    {
        tween.stop();
    }

    static void stop_and_finished(tween_action& tween)
    {
        tween.stop_and_finished_ = true;
    }

    static void stop_when_finished(tween_action& tween)
    {
		tween.stop_when_finished_ = true;
    }

    static void resume(tween_action& tween, bool force = false)
    {
        tween.resume({}, force);
    }

    static void resume(tween_action& tween, const std::string& key)
    {
        tween.resume(key);
    }

    static void pause(tween_action& tween)
    {
        tween.pause();
    }

    static void pause(tween_action& tween, const std::string& key)
    {
        tween.pause(key);
    }

    static void pause_forced(tween_action& tween)
    {
        tween.pause_forced();
    }

    static void pause_forced(tween_action& tween, const std::string& key)
    {
        tween.pause_forced(key);
    }

    static void set_speed_multiplier(tween_action& tween, float speed_multiplier)
	{
		speed_multiplier = std::min<float>(100.0f, speed_multiplier);
		speed_multiplier = std::max<float>(0.0f, speed_multiplier);
		tween.speed_multiplier_ = speed_multiplier;
	}

    static float get_speed_multiplier(const tween_action& tween)
	{
		return tween.speed_multiplier_;
	}

    static state_t get_state(const tween_action& tween)
    {
        return tween.state_;
    }

    static bool is_stop_when_finished_requested(const tween_action& tween)
	{
		return tween.stop_when_finished_;
	}

    static bool is_stop_and_finished_requested(const tween_action& tween)
    {
        return tween.stop_and_finished_;
    }

    static bool is_running(const tween_action& tween)
	{
		return tween.state_ == state_t::running;
	}

    static bool is_paused(const tween_action& tween)
	{
		return tween.state_ == state_t::paused;
	}

    static bool is_finished(const tween_action& tween)
    {
        return tween.state_ == state_t::finished;
    }

    static state_t update(tween_action& tween, duration_t delta)
	{
		return tween.update(delta);
	}

    static void set_elapsed(tween_action& self, duration_t elapsed)
    {
        self.set_elapsed(elapsed);
    }

    static void update_elapsed(tween_action& self, duration_t update_time)
    {
        self.update_elapsed(update_time);
    }

    static duration_t get_elapsed(const tween_action& self)
    {
        return self.elapsed_;
    }

    static duration_t get_duration(const tween_action& self)
    {
        return self.duration_;
    }

    static duration_t get_overflow(const tween_action& self)
    {
        auto duration = get_duration(self);
        auto elapsed_not_clamped = self.elapsed_not_clamped_;

        if(duration > duration_t::zero() && duration <= elapsed_not_clamped)
        {
            return elapsed_not_clamped - duration;
        }

        return {};
    }
};

} //end of namespace
