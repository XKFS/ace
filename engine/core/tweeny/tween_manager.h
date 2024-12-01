#pragma once
#include "tween_action.h"
#include <unordered_map>
#include <set>

namespace tweeny
{

struct tween_manager
{
    struct tween_info
    {
        tween_action tween;
        uint32_t depth = 0;
        std::vector<std::string> scopes;
    };
    // Use std::map so no iterators are invalidated
    using tween_collection = std::map<tween_id_t, tween_info>;

    tween_id_t start(tween_action tween, const tween_scope_policy& scope_policy = {});

    void stop(tween_id_t id);
    void stop_all(const std::string& scope = "");

    void pause(tween_id_t id);
    void pause_all(const std::string& scope = "", const std::string& key = "");

    void resume(tween_id_t id);
    void resume_all(const std::string& scope = "", const std::string& key = "");

    void stop_when_finished(tween_id_t id);
    void stop_when_finished_all(const std::string& scope = "");

    void stop_and_finish(tween_id_t id, duration_t finish_after = 0ms);
    void stop_and_finish_all(const std::string& scope = "");

    bool is_stopping(tween_id_t id) const;
    bool is_running(tween_id_t id) const;
    bool is_paused(tween_id_t id) const;
    bool is_finished(tween_id_t id) const;

    bool has_tween_with_scope(const std::string& scope_id);

	void set_speed_multiplier(tween_id_t id, float speed_multiplier = 1.0f);
	float get_speed_multiplier(tween_id_t id);

    duration_t get_elapsed(tween_id_t id) const;
    duration_t get_duration(tween_id_t id) const;
    duration_t get_overflow(tween_id_t id) const;
    void update(tween_id_t id, duration_t delta);

    // Dangerous function, proceed with caution!
    void set_elapsed(tween_id_t id, duration_t elapsed);

	void update(duration_t delta);

    void push_scope(const std::string& scope);
    void close_scope(const std::string& scope);
    void pop_scope();

    void clear_scopes();

    auto get_current_scope() const -> const std::string&;
    auto get_scopes() const -> const std::vector<std::string>&;
    auto get_tweenies() const -> const tween_collection&;
private:
    void start_tweeny(tween_info& tween_info);
    auto get_ids() const -> std::vector<tween_id_t>;

    std::vector<std::string> scopes_{};
    int32_t current_scope_idx_ = (-1);

    std::set<std::pair<std::string, std::string>> paused_scopes_{};

    tween_collection tweenies_{};
    tween_collection pending_tweenies_{};
};

} //end of namespace tweeny

//#include "tween_manager.hpp"
