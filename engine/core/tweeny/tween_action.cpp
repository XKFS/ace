#include "tween_action.h"
#include "tween_inspector.h"

namespace tweeny
{
tween_id_t unique_id = 1;

tween_action::tween_action(creator_t&& creator, duration_t duration, sentinel_t sentinel)
    : id_(unique_id++)
    , creator_(std::move(creator))
    , duration_(duration)
    , sentinel_(sentinel)
{
    if(duration_ < duration_t::zero())
    {
        duration_ = duration_t::zero();
    }
}

tween_id_t tween_action::get_id() const
{
    return id_;
}

bool tween_action::is_valid() const noexcept
{
    return get_id() > 0;
}

void tween_action::update_elapsed(duration_t update_time)
{
    elapsed_ += update_time;

    elapsed_not_clamped_ += update_time;

    clamp_elapsed();
}

void tween_action::set_elapsed(duration_t elapsed)
{
    elapsed_ = elapsed;
    elapsed_not_clamped_ = elapsed;

    clamp_elapsed();
}

void tween_action::clamp_elapsed()
{
    elapsed_ = std::max(elapsed_, duration_t::zero());
    elapsed_ = std::min(elapsed_, duration_);

    elapsed_not_clamped_ = std::max(elapsed_not_clamped_, duration_t::zero());
}

void tween_action::start()
{
    if(creator_ == nullptr)
    {
        return;
    }

    state_ = state_t::running;
    elapsed_ = duration_t::zero();
    elapsed_not_clamped_ = duration_t::zero();
    on_begin.emit();
    updater_ = creator_();
    state_ = updater_(0ms, *this);
    inspector::update_tween_state(*this, state_);

    if(state_ == state_t::finished)
    {
        if(false == sentinel_.expired())
        {
            on_end.emit();
        }
    }
}

void tween_action::stop()
{
    state_ = state_t::finished;
    inspector::update_tween_state(*this, state_);
}

void tween_action::resume(const std::string& key, bool force)
{
    if(state_ == state_t::paused)
    {
        if(force || pause_key_ == key)
        {
            pause_key_ = {};
            state_ = state_t::running;
            inspector::update_tween_state(*this, state_);
        }
    }
}

void tween_action::pause(const std::string& key)
{
    if(state_ == state_t::running)
    {
        pause_forced(key);
    }
}

void tween_action::pause_forced(const std::string& key)
{
    pause_key_ = key;

    pause_forced();
}

void tween_action::pause_forced()
{
    state_ = state_t::paused;
    inspector::update_tween_state(*this, state_);
}

state_t tween_action::update(duration_t delta)
{
    if(state_ == state_t::finished || state_ == state_t::paused)
    {
        return state_;
    }

    auto update_time = (delta.count() * int64_t(speed_multiplier_ * 1000.0f)) / int64_t(1000);
    if(updater_)
    {
        state_ = updater_(duration_t(update_time), *this);
    }
    else
    {
        stop();
    }

    if(state_ == state_t::finished)
    {
        inspector::update_tween_state(*this, state_);
        if(false == sentinel_.expired())
        {
            on_end.emit();
        }
    }
    else if(state_ == state_t::running)
    {
        on_update.emit();
    }
    return state_;
}

}
