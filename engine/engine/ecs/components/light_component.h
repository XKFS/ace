#pragma once
#include "basic_component.h"
#include <engine/rendering/light.h>

#include <base/basetypes.hpp>

namespace ace
{
//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : light_component (Class)
/// <summary>
/// Class that contains our core light data, used for rendering and other
/// things.
/// </summary>
//-----------------------------------------------------------------------------
class light_component : component_crtp<light_component>
{
public:
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //  Name : get_light ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_light() const -> const light&;

    //-----------------------------------------------------------------------------
    //  Name : set_light ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_light(const light& l);

    //-----------------------------------------------------------------------------
    //  Name : compute_projected_sphere_rect ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto compute_projected_sphere_rect(irect32_t& rect,
                                       const math::vec3& light_position,
                                       const math::vec3& light_direction,
                                       const math::transform& view,
                                       const math::transform& proj) -> int;

private:
    //-------------------------------------------------------------------------
    // Private Member Variables.
    //-------------------------------------------------------------------------
    /// The light object this component represents
    light light_;
};
} // namespace ace