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
    /// Specifies how forces are applied to physics components.
    /// </summary>
    public enum ForceMode : byte
    {
        /// <summary>
        /// Interprets the input as torque (measured in Newton-metres), and changes the angular velocity by the value of 
        /// torque * deltaTime / mass. The effect depends on the simulation step length and the mass of the body.
        /// </summary>
        Force,

        /// <summary>
        /// Interprets the parameter as angular acceleration (measured in degrees per second squared), and changes the
        /// angular velocity by the value of torque * deltaTime. The effect depends on the simulation step length but
        /// does not depend on the mass of the body.
        /// </summary>
        Acceleration,

        /// <summary>
        /// Interprets the parameter as angular momentum (measured in kilogram-meters-squared per second), and changes the
        /// angular velocity by the value of torque / mass. The effect depends on the mass of the body but doesn't depend 
        /// on the simulation step length.
        /// </summary>
        Impulse,

        /// <summary>
        /// Interprets the parameter as a direct angular velocity change (measured in degrees per second), and changes the
        /// angular velocity by the value of torque. The effect doesn't depend on the mass of the body or the simulation 
        /// step length.
        /// </summary>
        VelocityChange
    }

    /// <summary>
    /// Represents a contact point where a collision occurs.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct ContactPoint : IFormattable
    {
        private Vector3 point;
        private Vector3 normal;
        private float distance;
        private float impulse;

        /// <summary>
        /// Converts the contact point to its string representation.
        /// </summary>
        /// <returns>A string that represents the contact point.</returns>
        public override string ToString()
        {
            return ToString(null, null);
        }

        /// <summary>
        /// Converts the contact point to its string representation with a specified format.
        /// </summary>
        /// <param name="format">The format string.</param>
        /// <returns>A string that represents the contact point.</returns>
        public string ToString(string format)
        {
            return ToString(format, null);
        }

        /// <summary>
        /// Converts the contact point to its string representation with a specified format and format provider.
        /// </summary>
        /// <param name="format">The format string.</param>
        /// <param name="formatProvider">An object that supplies culture-specific formatting information.</param>
        /// <returns>A string that represents the contact point.</returns>
        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (string.IsNullOrEmpty(format))
            {
                format = "F2";
            }

            if (formatProvider == null)
            {
                formatProvider = CultureInfo.InvariantCulture.NumberFormat;
            }

            return string.Format(CultureInfo.InvariantCulture.NumberFormat, "(point={0}, normal={1}, distance={2}, impulse={3})",
                point.ToString(format, formatProvider),
                normal.ToString(format, formatProvider),
                distance.ToString(format, formatProvider),
                impulse.ToString(format, formatProvider));
        }
    };

    /// <summary>
    /// Represents a collision that occurs between two entities.
    /// </summary>
    public class Collision : IFormattable
    {
        /// <summary>
        /// Gets the entity involved in the collision.
        /// </summary>
        public Entity entity;

        /// <summary>
        /// Gets the array of contact points where the collision occurred.
        /// </summary>
        public ContactPoint[] contacts;

        /// <summary>
        /// Converts the collision to its string representation.
        /// </summary>
        /// <returns>A string that represents the collision.</returns>
        public override string ToString()
        {
            return ToString(null, null);
        }

        /// <summary>
        /// Converts the collision to its string representation with a specified format.
        /// </summary>
        /// <param name="format">The format string.</param>
        /// <returns>A string that represents the collision.</returns>
        public string ToString(string format)
        {
            return ToString(format, null);
        }

        /// <summary>
        /// Converts the collision to its string representation with a specified format and format provider.
        /// </summary>
        /// <param name="format">The format string.</param>
        /// <param name="formatProvider">An object that supplies culture-specific formatting information.</param>
        /// <returns>A string that represents the collision.</returns>
        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (string.IsNullOrEmpty(format))
            {
                format = "F2";
            }

            if (formatProvider == null)
            {
                formatProvider = CultureInfo.InvariantCulture.NumberFormat;
            }

            return string.Format(CultureInfo.InvariantCulture.NumberFormat, "(entity={0}, contacts={1})",
                entity.tag,
                string.Join(",",
                              contacts.Select(x => x.ToString()).ToArray()));
        }
    }

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
