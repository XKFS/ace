#include "model_system.h"
#include <engine/ecs/components/transform_component.h>
#include <engine/rendering/ecs/components/model_component.h>

#include <engine/ecs/ecs.h>
#include <engine/profiler/profiler.h>

#include <logging/logging.h>

#define POOLSTL_STD_SUPPLEMENT 1
#include <poolstl/poolstl.hpp>

namespace ace
{

auto model_system::init(rtti::context& ctx) -> bool
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto model_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_TRACE("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void model_system::on_frame_update(scene& scn, delta_t dt)
{
    APP_SCOPE_PERF("Model System");

    auto view = scn.registry->view<transform_component, model_component>();

    // this code should be thread safe as each task works with a whole hierarchy and
    // there is no interleaving between tasks.
    std::for_each(std::execution::par,
                  view.begin(),
                  view.end(),
                  [&](entt::entity entity)
                  {
                      auto& transform_comp = view.get<transform_component>(entity);
                      auto& model_comp = view.get<model_component>(entity);

                      // init
                      bool just_initted = model_comp.init_armature();

                      if(model_comp.was_used_last_frame() && !just_initted)
                      {
                          model_comp.update_armature();
                      }

                      model_comp.update_world_bounds(transform_comp.get_transform_global());
                  });
}

} // namespace ace
