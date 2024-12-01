#include <stdexcept>

#include "tween_manager.h"
#include "tween_private.h"

namespace tweeny
{

tween_id_t tween_manager::start(tween_action tween, const tween_scope_policy& scope_policy)
{
    const auto id = tween.get_id();

    if(is_running(tween))
    {
        return id;
    }

    if(is_paused(tween))
    {
        resume(tween);
        return id;
    }

    auto fill_info = [this, &scope_policy](tween_info& info, tween_action&& tween)
    {
        info.tween = std::move(tween);
        if(scope_policy.scope.empty())
        {
            info.scopes.insert(info.scopes.end(), scopes_.begin(), scopes_.end());
        }
        else
        {
            if(scope_policy.policy == tween_scope_policy::policy_t::independent)
            {
                info.scopes.emplace_back(scope_policy.scope);
            }
            else if(scope_policy.policy == tween_scope_policy::policy_t::stacked)
            {
                info.scopes.insert(info.scopes.end(), scopes_.begin(), scopes_.end());
                info.scopes.emplace_back(scope_policy.scope);
            }
        }
    };

    auto iter = tweenies_.find(id);
    if( iter == tweenies_.end() )
    {
        auto& info = tweenies_[id];
        fill_info(info, std::move(tween));

        start_tweeny(info);
    }
    else
    {
        // Occurs when start yourself from your own callback or start yourself from previous tween process
        auto& info = pending_tweenies_[id];
        fill_info(info, std::move(tween));
    }

    return id;
}

void tween_manager::stop(tween_id_t id)
{
    auto iter = tweenies_.find(id);
    if(iter != std::end(tweenies_))
    {
        tween_private::stop(iter->second.tween);
    }

    auto pending_iter = pending_tweenies_.find(id);
    if(pending_iter != std::end(pending_tweenies_))
    {
        pending_tweenies_.erase(pending_iter);
    }
}

void tween_manager::stop_all(const std::string& scope)
{
    for(auto& info : tweenies_)
    {
        if(std::find(info.second.scopes.begin(), info.second.scopes.end(), scope) != info.second.scopes.end())
        {
            tween_private::stop(info.second.tween);
        }
    }
}

void tween_manager::pause(const tween_id_t id)
{
    auto iter = tweenies_.find(id);
    if(iter != std::end(tweenies_))
    {
        tween_private::pause(iter->second.tween);
	}
}

void tween_manager::pause_all(const std::string& scope, const std::string& key)
{
    paused_scopes_.insert(std::make_pair(scope, key));

    for(auto& info : tweenies_)
    {
        if(std::find(info.second.scopes.begin(), info.second.scopes.end(), scope) != info.second.scopes.end())
        {
            tween_private::pause(info.second.tween, key);
        }
    }
}

void tween_manager::resume(const tween_id_t tween)
{
	auto iter = tweenies_.find(tween);
	if(iter != std::end(tweenies_))
	{
        tween_private::resume(iter->second.tween);
	}
}

void tween_manager::resume_all(const std::string& scope, const std::string& key)
{
    paused_scopes_.erase(std::make_pair(scope, key));

    for(auto& info : tweenies_)
    {
        if(std::find(info.second.scopes.begin(), info.second.scopes.end(), scope) != info.second.scopes.end())
        {
            tween_private::resume(info.second.tween, key);
        }
    }
}

void tween_manager::stop_when_finished(tween_id_t id)
{
	auto iter = tweenies_.find(id);
	if(iter != std::end(tweenies_))
	{
        tween_private::stop_when_finished(iter->second.tween);
	}
}

void tween_manager::stop_when_finished_all(const std::string& scope)
{
	for(auto& info : tweenies_)
	{
        if(std::find(info.second.scopes.begin(), info.second.scopes.end(), scope) != info.second.scopes.end())
        {
            tween_private::stop_when_finished(info.second.tween);
        }
	}
}

void tween_manager::stop_and_finish(tween_id_t id, duration_t /*finish_after*/)
{
    auto iter = tweenies_.find(id);
    if(iter == std::end(tweenies_))
    {
        return;
    }
    auto& tween = iter->second.tween;

    if(tween_private::get_state(tween) == state_t::finished)
    {
        return;
    }
    auto& depth = iter->second.depth;

    if(depth > 0)
    {
        stop(tween);
        throw std::runtime_error("Cannot call stop_and_finish from callbacks.");
    }

    tween_private::stop_and_finished(tween);
    tween_private::stop_when_finished(tween);
    while(true)
    {
        const bool force = true;
        tween_private::resume(tween, force);

        ++depth;
        bool finished = state_t::finished == tween_private::update(tween, 99h);
        --depth;

        if(finished)
        {
            break;
        }
    }
}

void tween_manager::stop_and_finish_all(const std::string& scope)
{
    for(auto& info : tweenies_)
    {
        if(std::find(info.second.scopes.begin(), info.second.scopes.end(), scope) != info.second.scopes.end())
        {
            stop_and_finish(info.second.tween);
        }
    }
}

bool tween_manager::is_stopping(tween_id_t id) const
{
	auto iter = tweenies_.find(id);
	if(iter != std::end(tweenies_))
	{
        return tween_private::is_stop_when_finished_requested(iter->second.tween);
	}
	return false;
}

bool tween_manager::is_running(tween_id_t id) const
{
	auto iter = tweenies_.find(id);
	if(iter != std::end(tweenies_))
	{
        return tween_private::is_running(iter->second.tween);
	}
	return false;
}

bool tween_manager::is_paused(tween_id_t id) const
{
	auto iter = tweenies_.find(id);
	if(iter != std::end(tweenies_))
	{
        return tween_private::is_paused(iter->second.tween);
	}
	return false;
}

bool tween_manager::is_finished(tween_id_t id) const
{
    auto iter = tweenies_.find(id);
    if(iter != std::end(tweenies_))
    {
        return tween_private::is_finished(iter->second.tween);
    }
    return true;
}

bool tween_manager::has_tween_with_scope(const std::string& scope_id)
{
    for(const auto& kvp : tweenies_)
    {
        if(std::find(kvp.second.scopes.begin(), kvp.second.scopes.end(), scope_id) != kvp.second.scopes.end())
        {
            return true;
        }
    };
    return false;
}

void tween_manager::set_speed_multiplier(tween_id_t id, float speed_multiplier)
{
	auto iter = tweenies_.find(id);
	if(iter != std::end(tweenies_))
	{
        tween_private::set_speed_multiplier(iter->second.tween, speed_multiplier);
	}
}

float tween_manager::get_speed_multiplier(tween_id_t id)
{
	auto iter = tweenies_.find(id);
	if(iter != std::end(tweenies_))
	{
        return tween_private::get_speed_multiplier(iter->second.tween);
	}
	return 1.0f;
}

duration_t tween_manager::get_elapsed(tween_id_t id) const
{
    auto iter = tweenies_.find(id);
    if(iter != std::end(tweenies_))
    {
        return tween_private::get_elapsed(iter->second.tween);
    }
    return {};
}

duration_t tween_manager::get_duration(tween_id_t id) const
{
    auto iter = tweenies_.find(id);
    if(iter != std::end(tweenies_))
    {
        return tween_private::get_duration(iter->second.tween);
    }
    return {};
}

duration_t tween_manager::get_overflow(tween_id_t id) const
{
    auto iter = tweenies_.find(id);
    if(iter != std::end(tweenies_))
    {
        return tween_private::get_overflow(iter->second.tween);
    }
    return {};
}

void tween_manager::update(tween_id_t id, duration_t delta)
{
    auto iter = tweenies_.find(id);
    if(iter != std::end(tweenies_))
    {
        tween_private::update(iter->second.tween, delta);
    }
}

void tween_manager::set_elapsed(tween_id_t id, duration_t elapsed)
{
    auto iter = tweenies_.find(id);
    if(iter == std::end(tweenies_))
    {
        return;
    }

    tween_private::set_elapsed(iter->second.tween, elapsed);
}

void tween_manager::update(duration_t delta)
{
    if(delta < duration_t::zero())
    {
        delta = duration_t::zero();
    }

    std::vector<tween_id_t> tweenies_to_start;
    tweenies_to_start.reserve(pending_tweenies_.size());

    for(auto& kvp : pending_tweenies_)
    {
        tweenies_to_start.emplace_back(kvp.first);

        tweenies_[kvp.first] = std::move(kvp.second);
    }
    pending_tweenies_.clear();

    for(auto id : tweenies_to_start)
    {
        start_tweeny(tweenies_.at(id));
    }

    for(auto id : get_ids())
    {
        auto& tween = tweenies_[id];

        if(tween.depth > 0)
        {
            continue;
        }

        tween.depth++;

        auto state = tween_private::update(tween.tween, delta);

        tween.depth--;

        if(state == state_t::finished)
        {
            tweenies_.erase(tween.tween.get_id());
        }
    }
}

void tween_manager::push_scope(const std::string& scope)
{
    if(scope.empty())
    {
        return;
    }

    auto iter = std::find(scopes_.begin(), scopes_.end(), scope);
    if(iter != scopes_.end())
    {
        throw std::logic_error("push_scope that is already pushed.");
    }

    scopes_.emplace_back(scope);
    current_scope_idx_ = static_cast<int32_t>(scopes_.size()) - 1;
}

void tween_manager::pop_scope()
{
    if(current_scope_idx_ < int32_t(scopes_.size()))
    {
        close_scope(scopes_.at(current_scope_idx_));
    }
}

void tween_manager::close_scope(const std::string& scope)
{
    auto iter = std::find(scopes_.begin(), scopes_.end(), scope);
    scopes_.erase(iter, scopes_.end());

    current_scope_idx_ = static_cast<int32_t>(scopes_.size()) - 1;
}

void tween_manager::clear_scopes()
{
    scopes_.clear();
    current_scope_idx_ = (-1);
}

void tween_manager::start_tweeny(tween_info& tween_info)
{
    auto& tween = tween_info.tween;

    bool paused_by_scope = false;
    // First force pause the tweeny before it was started
    for(const auto& scope : tween_info.scopes)
    {
        auto it = std::find_if(paused_scopes_.begin(), paused_scopes_.end(), [&](const auto& sc)
                               {
                                   return sc.first == scope;
                               });
        if(it != paused_scopes_.end())
        {
            const auto& scope_pause_key = it->second;
            tween_private::pause_forced(tween, scope_pause_key);
            paused_by_scope = true;
            break;
        }
    }

    tween_private::start(tween);

    if(paused_by_scope)
    {
        tween_private::pause_forced(tween);
    }
}

const std::string& tween_manager::get_current_scope() const
{
    static const std::string default_scope{};
    if(scopes_.empty())
    {
        return default_scope;
    }
    return scopes_.at(current_scope_idx_);
}

const std::vector<std::string>& tween_manager::get_scopes() const
{
    return scopes_;
}

const tween_manager::tween_collection& tween_manager::get_tweenies() const
{
    return tweenies_;
}

std::vector<tween_id_t> tween_manager::get_ids() const
{
    std::vector<tween_id_t> result;
    result.reserve(tweenies_.size());
    for(const auto& kvp : tweenies_)
    {
        result.emplace_back(kvp.first);
    }
    return result;
}


} //namespace tweeny
