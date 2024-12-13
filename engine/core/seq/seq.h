#pragma once
#include "seq_core.h"
#include "seq_manager.h"
#include <hpp/source_location.hpp>

namespace seq
{
auto start(seq_action action,
           const seq_scope_policy& scope_policy = {},
           hpp::source_location location = hpp::source_location::current()) -> seq_id_t;

void stop(seq_id_t id);
void pause(seq_id_t id);
void resume(seq_id_t id);
void stop_when_finished(seq_id_t id);
void stop_and_finish(seq_id_t id, duration_t finish_after = 0ms);
void update(duration_t delta);
void update(duraiton_secs_t delta);
void shutdown();
auto is_stopping(seq_id_t id) -> bool;
auto is_running(seq_id_t id) -> bool;
auto is_paused(seq_id_t id) -> bool;
auto is_finished(seq_id_t id) -> bool;
auto has_action_with_scope(const std::string& scope_id) -> bool;

void set_speed_multiplier(seq_id_t id, float speed_multiplier = 1.0f);
auto get_speed_multiplier(seq_id_t id) -> float;

auto get_elapsed(seq_id_t id) -> duration_t;
void set_elapsed(seq_id_t id, duration_t duration);
void update(seq_id_t id, duration_t duration);

auto get_duration(seq_id_t id) -> duration_t;
auto get_overflow(seq_id_t id) -> duration_t;

auto get_percent(seq_id_t id) -> float;

namespace manager
{
void push(seq_manager& mgr);
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

} // namespace seq
