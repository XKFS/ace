#pragma once
#include "seq_common.h"
#include <hpp/event.hpp>

namespace seq
{

struct seq_action
{
    friend struct seq_private;

    using updater_t = std::function<state_t(duration_t, seq_action&)>;
    using creator_t = std::function<updater_t()>;

    seq_action() = default;
    ~seq_action() = default;
    seq_action(creator_t&& creator, duration_t duration, sentinel_t sentinel);

    seq_action(const seq_action&) = default;
    seq_action(seq_action&&) = default;

    auto operator=(const seq_action&) -> seq_action& = default;
    auto operator=(seq_action&&) -> seq_action& = default;

    //---------------------------------------------------------------------------------------
    /// on_begin - Emit when action is started.
    //---------------------------------------------------------------------------------------
    hpp::event<void()> on_begin;

    //---------------------------------------------------------------------------------------
    /// on_step - Emit every time the action value is changed.
    //---------------------------------------------------------------------------------------
    hpp::event<void()> on_step;

    //---------------------------------------------------------------------------------------
    /// on_update - Emit every frame when state of action is running.
    //---------------------------------------------------------------------------------------
    hpp::event<void()> on_update;

    //---------------------------------------------------------------------------------------
    /// on_end - Emit when action is finished, if stop is called we consider it
    /// as interupt and on_end is not called.
    //---------------------------------------------------------------------------------------
    hpp::event<void()> on_end;

    seq_inspect_info::ptr info;

    auto get_id() const -> seq_id_t;

    operator seq_id_t() const;

    auto is_valid() const noexcept -> bool;

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

    auto update(duration_t delta) -> state_t;

    seq_id_t id_{};
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

} // namespace seq
