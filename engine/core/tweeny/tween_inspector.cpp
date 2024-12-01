#include "tween_inspector.h"

namespace tweeny
{
namespace inspector
{
void add_sequence_info(tween_action& action, const std::vector<tween_action>& tweenies)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    tween_inspect_info::ptr info = std::make_shared<tween_inspect_info>();
    info->id = action.get_id();
    info->updater_type = "sequence";
    info->modified_type = "tween_action";

    info->begin_value = "0";
    info->current_value = "0";
    info->end_value = to_str(tweenies.size());

    info->duration = tween_private::get_duration(action);

    for(const auto& tween : tweenies)
    {
        if(tween.info)
        {
            info->children.emplace_back(tween.info);
        }
    }

    action.info = std::move(info);
#endif
}

void add_together_info(tween_action& action, const std::vector<tween_action>& tweenies)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    tween_inspect_info::ptr info = std::make_shared<tween_inspect_info>();
    info->id = action.get_id();
    info->updater_type = "together";
    info->modified_type = "tween_action";

    info->begin_value = "n/a";
    info->current_value = "n/a";
    info->end_value = "n/a";

    info->duration = tween_private::get_duration(action);

    for(const auto& tween : tweenies)
    {
        if(tween.info)
        {
            info->children.emplace_back(tween.info);
        }
    }

    action.info = std::move(info);
#endif
}

void add_delay_info(tween_action& action)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    tween_inspect_info::ptr info = std::make_shared<tween_inspect_info>();
    info->id = action.get_id();
    info->updater_type = "delay";
    info->modified_type = "n/a";

    info->begin_value = "n/a";
    info->current_value = "n/a";
    info->end_value = "n/a";

    info->duration = tween_private::get_duration(action);

    action.info = std::move(info);
#endif
}

void add_repeat_info(tween_action& repeat_action, const tween_action& action, size_t times)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    tween_inspect_info::ptr info = std::make_shared<tween_inspect_info>();
    info->id = repeat_action.get_id();
    info->updater_type = "repeat";

    if(times == 0)
    {
        info->begin_value = "infinity";
        info->current_value = "infinity";
        info->end_value = "infinity";
        info->duration = 0ms;
    }
    else
    {
        info->begin_value = "0";
        info->current_value = "0";
        info->end_value = std::to_string(times);
        info->duration = duration_t(tween_private::get_duration(repeat_action).count() * times);
    }

    if(action.info)
    {
        info->children.emplace_back(action.info);
    }
    repeat_action.info = std::move(info);
#endif
}

void update_tween_state(tween_action& action, state_t state)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    if(action.info)
    {
        action.info->state = to_str(state);
    }
#endif
}

void update_tween_status(tween_action& action)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    if(action.info)
    {
        update_tween_status(action, 0);
        action.info->current_value = "n/a";
    }
#endif
}

void add_location(tween_action& action, const hpp::source_location& location)
{
#ifdef TWEENY_INSPECTOR_ENABLE
    if(action.info)
    {
        action.info->file_name = location.file_name();
        action.info->function_name = location.function_name();
        action.info->line_number = location.line();
        action.info->column_offset = location.column();
    }
#endif
}

} //end of namespace inspector
} //end of namespace tweeny
