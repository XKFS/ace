using System;
using System.Globalization;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{

    /// <summary>
    /// Provides physics functionality for an entity.
    /// </summary>
    public class PhysicsComponent : Component
    {
    

        /// <summary>
        /// Applies an explosion force to the entity.
        /// </summary>
        /// <param name="explosionForce">The force of the explosion.</param>
        /// <param name="explosionPosition">The center of the explosion.</param>
        /// <param name="explosionRadius">The radius of the explosion.</param>
        /// <param name="upwardsModifier">Adjusts the upward direction of the explosion force.</param>
        /// <param name="mode">The force mode to apply.</param>
        public void ApplyExplosionForce(float explosionForce, Vector3 explosionPosition, float explosionRadius, float upwardsModifier = 0.0f, ForceMode mode = ForceMode.Force)
        {
            internal_m2n_apply_explosion_force(owner, explosionForce, explosionPosition, explosionRadius, upwardsModifier, mode);
        }

        /// <summary>
        /// Applies a force to the entity.
        /// </summary>
        /// <param name="force">The force to apply.</param>
        /// <param name="mode">The force mode to apply.</param>
        public void ApplyForce(Vector3 force, ForceMode mode = ForceMode.Force)
        {
            internal_m2n_apply_force(owner, force, mode);
        }

        /// <summary>
        /// Applies a torque to the entity.
        /// </summary>
        /// <param name="torque">The torque to apply.</param>
        /// <param name="mode">The force mode to apply.</param>
        public void ApplyTorque(Vector3 torque, ForceMode mode = ForceMode.Force)
        {
            internal_m2n_apply_torque(owner, torque, mode);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void internal_m2n_apply_explosion_force(Entity eid, float explosionForce, Vector3 explosionPosition, float explosionRadius, float upwardsModifier, ForceMode mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void internal_m2n_apply_force(Entity eid, Vector3 force, ForceMode mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void internal_m2n_apply_torque(Entity eid, Vector3 torque, ForceMode mode);
    }
}
}
