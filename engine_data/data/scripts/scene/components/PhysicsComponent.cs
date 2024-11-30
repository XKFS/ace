using System;
using System.Globalization;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{

public enum ForceMode : byte
{
    // Interprets the input as torque (measured in Newton-metres), and changes the angular velocity by the value of
    // torque * DT / mass. The effect depends on the simulation step length and the mass of the body.
    Force,

    // Interprets the parameter as angular acceleration (measured in degrees per second squared), and changes the
    // angular velocity by the value of torque * DT. The effect depends on the simulation step length but does not
    // depend on the mass of the body.
    Acceleration,

    // Interprets the parameter as an angular momentum (measured in kilogram-meters-squared per second), and changes the
    // angular velocity by the value of torque / mass. The effect depends on the mass of the body but doesn't depend on
    // the simulation step length.
    Impulse,
    
    // Interprets the parameter as a direct angular velocity change (measured in degrees per second), and changes the
    // angular velocity by the value of torque. The effect doesn't depend on the mass of the body and the simulation
    // step length.
    VelocityChange
}


[StructLayout(LayoutKind.Sequential)]
public struct ContactPoint : IFormattable
{
    Vector3 point;
    Vector3 normal;
    float distance;
    float impulse;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override string ToString()
    {
        return ToString(null, null);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public string ToString(string format)
    {
        return ToString(format, null);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
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

public class Collision : IFormattable
{
    public Entity entity;
    public ContactPoint[] contacts;

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override string ToString()
    {
        return ToString(null, null);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public string ToString(string format)
    {
        return ToString(format, null);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
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

public class PhysicsComponent : Component
{
    public void ApplyExplosionForce(float explosionForce, Vector3 explosionPosition, float explosionRadius, float upwardsModifier = 0.0f, ForceMode mode = ForceMode.Force)
    {
        internal_m2n_apply_explosion_force(owner, explosionForce, explosionPosition, explosionRadius, upwardsModifier, mode);
    }

    public void ApplyForce(Vector3 force, ForceMode mode = ForceMode.Force)
    {
        internal_m2n_apply_force(owner, force, mode);
    }

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


