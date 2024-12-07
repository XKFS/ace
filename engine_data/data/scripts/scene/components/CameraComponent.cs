using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public class CameraComponent : Component
{
    public bool ViewportToRay(Vector2 pos, out Ray ray)
    {
        return internal_m2n_camera_viewport_to_ray(owner, pos, out ray);
    }


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool internal_m2n_camera_viewport_to_ray(Entity eid, Vector2 pos, out Ray ray);
}

}
}


