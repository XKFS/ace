using System;
using System.Globalization;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{
    public static class Physics
    {
        public const int DefaultRaycastLayers = -1;
        public static RaycastHit? Raycast(Ray ray, float maxDistance = Mathf.Infinity, int layerMask = DefaultRaycastLayers, bool querySensors = false)
        {
            return Raycast(ray.origin, ray.direction, maxDistance, layerMask, querySensors);
        }
        public static RaycastHit? Raycast(Vector3 origin, Vector3 direction, float maxDistance = Mathf.Infinity, int layerMask = DefaultRaycastLayers, bool querySensors = false)
        {
            RaycastHit hit;
            bool hasHit = internal_m2n_physics_raycast(out hit, origin, direction, maxDistance, layerMask, querySensors);
            if(hasHit)
            {
                return hit;
            }
            return null;
        }
        public static RaycastHit[] RaycastAll(Ray ray, float maxDistance = Mathf.Infinity, int layerMask = DefaultRaycastLayers, bool querySensors = false)
        {
            return RaycastAll(ray.origin, ray.direction, maxDistance, layerMask, querySensors);
        }

        public static RaycastHit[] RaycastAll(Vector3 origin, Vector3 direction, float maxDistance = Mathf.Infinity, int layerMask = DefaultRaycastLayers, bool querySensors = false)
        {
            byte[] rawHits = internal_m2n_physics_raycast_all(origin, direction, maxDistance, layerMask, querySensors);
            return rawHits.ToStructArray<RaycastHit>();
        }

        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool internal_m2n_physics_raycast(out RaycastHit hit, Vector3 origin, Vector3 direction, float maxDistance, int layerMask, bool querySensors);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern byte[] internal_m2n_physics_raycast_all(Vector3 origin, Vector3 direction, float maxDistance, int layerMask, bool querySensors);
    }
}
}
