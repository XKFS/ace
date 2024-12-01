#pragma once
#include "tween_common.h"
#include "tween_action.h"
#include "tween_private.h"
#include <hpp/source_location.hpp>

namespace tweeny
{
namespace inspector
{

template<typename T>
inline void update_begin_value(tween_action& action, const T& begin)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    if(action.info)
    {
        action.info->begin_value = to_str(begin);
    }
#endif
}

template<typename T>
inline void update_tween_status(tween_action& action, const T& current)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    if(action.info)
    {
        action.info->current_value = to_str(current);
        action.info->elapsed = tween_private::get_elapsed(action);
        action.info->progress = float(action.info->elapsed.count()) / float(action.info->duration.count());
        action.info->speed_multiplier = tween_private::get_speed_multiplier(action);
        action.info->stop_when_finished = tween_private::is_stop_when_finished_requested(action);
    }
#endif
}

template<typename Object, typename T>
inline void add_info(tween_action& action,
                     const std::string& updater_type,
                     const Object& object,
                     const T& end_value,
                     const ease_t& ease_func)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    action.info = std::make_shared<tween_inspect_info>();
    action.info->id = action.get_id();
    action.info->duration = tween_private::get_duration(action);
    action.info->updater_type = updater_type;
    action.info->modified_type = type_to_str(object);
    action.info->end_value = to_str(end_value);
    action.info->ease_func = ease_func;
#endif
}

void update_tween_status(tween_action& action);
void update_tween_state(tween_action& action, state_t state);

void add_sequence_info(tween_action& action, const std::vector<tween_action>& tweenies);
void add_together_info(tween_action& action, const std::vector<tween_action>& tweenies);
void add_delay_info(tween_action& action);
void add_repeat_info(tween_action& repeat_action, const tween_action& action, size_t times = 0);
void add_location(tween_action& action, const hpp::source_location& location);

} //end of namespace inspector
} //end of namespace tweeny
