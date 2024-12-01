#include "tweeny.h"
#include "detail/tweeny_internal.h"
#include "tween_inspector.h"

namespace tweeny
{

auto start(tween_action tween, const tween_scope_policy& scope_policy, hpp::source_location location) -> tween_id_t
{
    inspector::add_location(tween, location);
    return detail::get_manager().start(std::move(tween), scope_policy);
}

void stop(tween_id_t id)
{
    detail::get_manager().stop(id);
}

void pause(const tween_id_t id)
{
    detail::get_manager().pause(id);
}

void resume(const tween_id_t id)
{
    detail::get_manager().resume(id);
}

void stop_when_finished(tween_id_t id)
{
    detail::get_manager().stop_when_finished(id);
}

void stop_and_finish(tween_id_t id, duration_t finish_after)
{
    detail::get_manager().stop_and_finish(id, finish_after);
}

auto is_stopping(tween_id_t id) -> bool
{
    return detail::get_manager().is_stopping(id);
}

auto is_running(tween_id_t id) -> bool
{
    return detail::get_manager().is_running(id);
}

auto is_paused(tween_id_t id) -> bool
{
    return detail::get_manager().is_paused(id);
}

auto is_finished(tween_id_t id) -> bool
{
    return detail::get_manager().is_finished(id);
}

auto has_tween_with_scope(const std::string& scope_id) -> bool
{
    return detail::get_manager().has_tween_with_scope(scope_id);
}

void set_speed_multiplier(tween_id_t id, float speed_multiplier)
{
    return detail::get_manager().set_speed_multiplier(id, speed_multiplier);
}

auto get_speed_multiplier(tween_id_t id) -> float
{
    return detail::get_manager().get_speed_multiplier(id);
}

auto get_elapsed(tween_id_t id) -> duration_t
{
    return detail::get_manager().get_elapsed(id);
}

void set_elapsed(tween_id_t id, duration_t duration)
{
    return detail::get_manager().set_elapsed(id, duration);
}

auto get_duration(tween_id_t id) -> duration_t
{
    return detail::get_manager().get_duration(id);
}

auto get_overflow(tween_id_t id) -> duration_t
{
    return detail::get_manager().get_overflow(id);
}

void update(tween_id_t id, duration_t delta)
{
    detail::get_manager().update(id, delta);
}

auto get_percent(tween_id_t id) -> float
{
    auto dur = get_duration(id);
    if(dur == duration_t::zero())
    {
        return 0.0f;
    }
    float elapsed = float(get_elapsed(id).count());
    float total_dur = float(dur.count());
    return elapsed / total_dur;
}

void update(duration_t delta)
{
    detail::get_manager().update(delta);
}

void shutdown()
{
    detail::get_manager() = {};
}

namespace scope
{
void push(const std::string& scope)
{
    detail::get_manager().push_scope(scope);
}

void pop()
{
    detail::get_manager().pop_scope();
}

void close(const std::string& scope)
{
    detail::get_manager().close_scope(scope);
}

void clear()
{
    detail::get_manager().clear_scopes();
}

auto get_current() -> const std::string&
{
    return detail::get_manager().get_current_scope();
}

void stop_all(const std::string& scope)
{
    detail::get_manager().stop_all(scope);
}

void pause_all(const std::string& scope)
{
    detail::get_manager().pause_all(scope);
}

void resume_all(const std::string& scope)
{
    detail::get_manager().resume_all(scope);
}

void pause_all(const std::string& scope, const std::string& key)
{
    detail::get_manager().pause_all(scope, key);
}

void resume_all(const std::string& scope, const std::string& key)
{
    detail::get_manager().resume_all(scope, key);
}

void stop_and_finish_all(const std::string& scope)
{
    detail::get_manager().stop_and_finish_all(scope);
}

void stop_when_finished_all(const std::string& scope)
{
    detail::get_manager().stop_when_finished_all(scope);
}

} // namespace scope

namespace manager
{
void push(tween_manager& mgr)
{
    detail::push(mgr);
}
void pop()
{
    detail::pop();
}
} // namespace manager
} // namespace tweeny
