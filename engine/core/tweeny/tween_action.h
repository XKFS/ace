#pragma once
#include "tween_common.h"
#include <hpp/event.hpp>

namespace tweeny
{

struct tween_action
{
    friend struct tween_private;

    using updater_t = std::function<state_t(duration_t, tween_action&)>;
    using creator_t = std::function<updater_t()>;

    tween_action() = default;
    ~tween_action() = default;
    tween_action(creator_t&& creator, duration_t duration, sentinel_t sentinel);

    tween_action(const tween_action&) = default;
    tween_action(tween_action&&) = default;

    tween_action& operator=(const tween_action&) = default;
    tween_action& operator=(tween_action&&) = default;

    //---------------------------------------------------------------------------------------
    /// on_begin - Emit when tween is started.
    //---------------------------------------------------------------------------------------
    hpp::event<void()> on_begin;

    //---------------------------------------------------------------------------------------
    /// on_step - Emit every time the tweened value is changed.
    //---------------------------------------------------------------------------------------
    hpp::event<void()> on_step;

    //---------------------------------------------------------------------------------------
    /// on_update - Emit every frame when state of tween is running.
    //---------------------------------------------------------------------------------------
    hpp::event<void()> on_update;

    //---------------------------------------------------------------------------------------
    /// on_end - Emit when tween is finished, if stop is called we consider it
    /// as interupt and on_end is not called.
    //---------------------------------------------------------------------------------------
    hpp::event<void()> on_end;

    tween_inspect_info::ptr info;

    tween_id_t get_id() const;
    inline operator tween_id_t() const
    {
        return get_id();
    }

    bool is_valid() const noexcept;
private:
    void update_elapsed(duration_t update_time);
    void set_elapsed(duration_t elapsed);
    void clamp_elapsed();

    void start();
    void stop();

    void resume(const std::string& key = "", bool force = false);
    void pause(const std::string& key = "");

    void pause_forced(const std::string& key);
    void pause_forced();

    state_t update(duration_t delta);
private:
    tween_id_t id_ {};
    creator_t creator_;
    updater_t updater_;

    std::string pause_key_{};
    state_t state_ = state_t::finished;

    duration_t elapsed_{};
    duration_t elapsed_not_clamped_{};
    duration_t duration_{};

    sentinel_t sentinel_;

    bool stop_and_finished_ = false;
    bool stop_when_finished_ = false;
    float speed_multiplier_ = 1.0f;
};

}
