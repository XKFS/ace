#pragma once
#include "seq_action.h"

namespace seq
{

struct seq_private
{
    static void start(seq_action& tween)
    {
        tween.start();
    }

    static void stop(seq_action& tween)
    {
        tween.stop();
    }

    static void stop_and_finished(seq_action& tween)
    {
        tween.stop_and_finished_ = true;
    }

    static void stop_when_finished(seq_action& tween)
    {
        tween.stop_when_finished_ = true;
    }

    static void resume(seq_action& tween, bool force = false)
    {
        tween.resume({}, force);
    }

    static void resume(seq_action& tween, const std::string& key)
    {
        tween.resume(key);
    }

    static void pause(seq_action& tween)
    {
        tween.pause();
    }

    static void pause(seq_action& tween, const std::string& key)
    {
        tween.pause(key);
    }

    static void pause_forced(seq_action& tween)
    {
        tween.pause_forced();
    }

    static void pause_forced(seq_action& tween, const std::string& key)
    {
        tween.pause_forced(key);
    }

    static void set_speed_multiplier(seq_action& tween, float speed_multiplier)
    {
        speed_multiplier = std::min<float>(100.0f, speed_multiplier);
        speed_multiplier = std::max<float>(0.0f, speed_multiplier);
        tween.speed_multiplier_ = speed_multiplier;
    }

    static auto get_speed_multiplier(const seq_action& tween) -> float
    {
        return tween.speed_multiplier_;
    }

    static auto get_state(const seq_action& tween) -> state_t
    {
        return tween.state_;
    }

    static auto is_stop_when_finished_requested(const seq_action& tween) -> bool
    {
        return tween.stop_when_finished_;
    }

    static auto is_stop_and_finished_requested(const seq_action& tween) -> bool
    {
        return tween.stop_and_finished_;
    }

    static auto is_running(const seq_action& tween) -> bool
    {
        return tween.state_ == state_t::running;
    }

    static auto is_paused(const seq_action& tween) -> bool
    {
        return tween.state_ == state_t::paused;
    }

    static auto is_finished(const seq_action& tween) -> bool
    {
        return tween.state_ == state_t::finished;
    }

    static auto update(seq_action& tween, duration_t delta) -> state_t
    {
        return tween.update(delta);
    }

    static void set_elapsed(seq_action& self, duration_t elapsed)
    {
        self.set_elapsed(elapsed);
    }

    static void update_elapsed(seq_action& self, duration_t update_time)
    {
        self.update_elapsed(update_time);
    }

    static auto get_elapsed(const seq_action& self) -> duration_t
    {
        return self.elapsed_;
    }

    static auto get_duration(const seq_action& self) -> duration_t
    {
        return self.duration_;
    }

    static auto get_overflow(const seq_action& self) -> duration_t
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

} // namespace seq
