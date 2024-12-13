#pragma once
#include "seq_action.h"
#include <set>
#include <unordered_map>

namespace seq
{

struct seq_manager
{
    struct seq_info
    {
        seq_action action;
        uint32_t depth = 0;
        std::vector<std::string> scopes;
    };
    // Use std::map so no iterators are invalidated
    using action_collection_t = std::map<seq_id_t, seq_info>;

    seq_id_t start(seq_action action, const seq_scope_policy& scope_policy = {});

    void stop(seq_id_t id);
    void stop_all(const std::string& scope = {});

    void pause(seq_id_t id);
    void pause_all(const std::string& scope = {}, const std::string& key = {});

    void resume(seq_id_t id);
    void resume_all(const std::string& scope = {}, const std::string& key = {});

    void stop_when_finished(seq_id_t id);
    void stop_when_finished_all(const std::string& scope = {});

    void stop_and_finish(seq_id_t id, duration_t finish_after = 0ms);
    void stop_and_finish_all(const std::string& scope = {});

    auto is_stopping(seq_id_t id) const -> bool;
    auto is_running(seq_id_t id) const -> bool;
    auto is_paused(seq_id_t id) const -> bool;
    auto is_finished(seq_id_t id) const -> bool;

    bool has_action_with_scope(const std::string& scope_id);
    void set_speed_multiplier(seq_id_t id, float speed_multiplier = 1.0f);
    auto get_speed_multiplier(seq_id_t id) -> float;

    auto get_elapsed(seq_id_t id) const -> duration_t;
    auto get_duration(seq_id_t id) const -> duration_t;
    auto get_overflow(seq_id_t id) const -> duration_t;
    void update(seq_id_t id, duration_t delta);

    // Dangerous function, proceed with caution!
    void set_elapsed(seq_id_t id, duration_t elapsed);

    void update(duration_t delta);

    void push_scope(const std::string& scope);
    void close_scope(const std::string& scope);
    void pop_scope();

    void clear_scopes();

    auto get_current_scope() const -> const std::string&;
    auto get_scopes() const -> const std::vector<std::string>&;
    auto get_actions() const -> const action_collection_t&;

private:
    void start_action(seq_info& info);
    auto get_ids() const -> std::vector<seq_id_t>;

    std::vector<std::string> scopes_;
    int32_t current_scope_idx_ = (-1);

    std::set<std::pair<std::string, std::string>> paused_scopes_;

    action_collection_t actions_;
    action_collection_t pending_actions_;
};

} // namespace seq

// #include "seq_manager.hpp"
