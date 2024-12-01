#pragma once
#include "tween_core.h"
#include "tween_manager.h"
#include <hpp/source_location.hpp>

namespace tweeny
{
auto start(tween_action tween,
           const tween_scope_policy& scope_policy = {},
           hpp::source_location location = hpp::source_location::current()) -> tween_id_t;

void stop(tween_id_t id);
void pause(tween_id_t id);
void resume(tween_id_t id);
void stop_when_finished(tween_id_t id);
void stop_and_finish(tween_id_t id, duration_t finish_after = 0ms);
void update(duration_t delta);
void shutdown();
auto is_stopping(tween_id_t id) -> bool;
auto is_running(tween_id_t id) -> bool;
auto is_paused(tween_id_t id) -> bool;
auto is_finished(tween_id_t id) -> bool;
auto has_tween_with_scope(const std::string& scope_id) -> bool;

void set_speed_multiplier(tween_id_t id, float speed_multiplier = 1.0f);
auto get_speed_multiplier(tween_id_t id) -> float;

auto get_elapsed(tween_id_t id) -> duration_t;
void set_elapsed(tween_id_t id, duration_t duration);
void update(tween_id_t id, duration_t duration);

auto get_duration(tween_id_t id) -> duration_t;
auto get_overflow(tween_id_t id) -> duration_t;

auto get_percent(tween_id_t id) -> float;

namespace manager
{
void push(tween_manager& mgr);
void pop();
} // namespace manager

namespace scope
{
void push(const std::string& scope);
void close(const std::string& scope);
void pop();

void clear();
auto get_current() -> const std::string&;

void stop_all(const std::string& scope);
void pause_all(const std::string& scope);
void resume_all(const std::string& scope);
void pause_all(const std::string& scope, const std::string& key);
void resume_all(const std::string& scope, const std::string& key);
void stop_and_finish_all(const std::string& scope);
void stop_when_finished_all(const std::string& scope);
} // end of namespace scope

} // end of namespace tweeny
