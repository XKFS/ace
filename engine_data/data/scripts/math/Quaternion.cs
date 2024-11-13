using System;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct Quaternion : IEquatable<Quaternion>, IFormattable
{
    //
    // Summary:
    //     X component of the Quaternion. Don't modify this directly unless you know quaternions
    //     inside out.
    public float x;

    //
    // Summary:
    //     Y component of the Quaternion. Don't modify this directly unless you know quaternions
    //     inside out.
    public float y;

    //
    // Summary:
    //     Z component of the Quaternion. Don't modify this directly unless you know quaternions
    //     inside out.
    public float z;

    //
    // Summary:
    //     W component of the Quaternion. Do not directly modify quaternions.
    public float w;

    private static readonly Quaternion identityQuaternion = new Quaternion(0f, 0f, 0f, 1f);

    //
    // Summary:
    //     The identity rotation (Read Only).
    public static Quaternion identity
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return identityQuaternion;
        }
    }

    //
    // Summary:
    //     Returns or sets the euler angle representation of the rotation.
    public Vector3 eulerAngles
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return Internal_MakePositive(Internal_ToEulerRad(this) * 57.29578f);
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        set
        {
            this = Internal_FromEulerRad(value * (MathF.PI / 180f));
        }
    }

    //
    // Summary:
    //     Returns this quaternion with a magnitude of 1 (Read Only).
    public Quaternion normalized
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return Normalize(this);
        }
    }

    //
    // Summary:
    //     Creates a rotation which rotates from fromDirection to toDirection.
    //
    // Parameters:
    //   fromDirection:
    //
    //   toDirection:
    //[FreeFunction("FromToQuaternionSafe", IsThreadSafe = true)]
    public static Quaternion FromToRotation(Vector3 fromDirection, Vector3 toDirection)
    {
        FromToRotation_Injected(ref fromDirection, ref toDirection, out var ret);
        return ret;
    }

    //
    // Summary:
    //     Returns the Inverse of rotation.
    //
    // Parameters:
    //   rotation:
    //[FreeFunction(IsThreadSafe = true)]
    public static Quaternion Inverse(Quaternion rotation)
    {
        Inverse_Injected(ref rotation, out var ret);
        return ret;
    }

    //
    // Summary:
    //     Spherically interpolates between quaternions a and b by ratio t. The parameter
    //     t is clamped to the range [0, 1].
    //
    // Parameters:
    //   a:
    //     Start value, returned when t = 0.
    //
    //   b:
    //     End value, returned when t = 1.
    //
    //   t:
    //     Interpolation ratio.
    //
    // Returns:
    //     A quaternion spherically interpolated between quaternions a and b.
    //[FreeFunction("QuaternionScripting::Slerp", IsThreadSafe = true)]
    public static Quaternion Slerp(Quaternion a, Quaternion b, float t)
    {
        Slerp_Injected(ref a, ref b, t, out var ret);
        return ret;
    }

    //
    // Summary:
    //     Spherically interpolates between a and b by t. The parameter t is not clamped.
    //
    //
    // Parameters:
    //   a:
    //
    //   b:
    //
    //   t:
    //[FreeFunction("QuaternionScripting::SlerpUnclamped", IsThreadSafe = true)]
    public static Quaternion SlerpUnclamped(Quaternion a, Quaternion b, float t)
    {
        SlerpUnclamped_Injected(ref a, ref b, t, out var ret);
        return ret;
    }

    //
    // Summary:
    //     Interpolates between a and b by t and normalizes the result afterwards. The parameter
    //     t is clamped to the range [0, 1].
    //
    // Parameters:
    //   a:
    //     Start value, returned when t = 0.
    //
    //   b:
    //     End value, returned when t = 1.
    //
    //   t:
    //     Interpolation ratio.
    //
    // Returns:
    //     A quaternion interpolated between quaternions a and b.
    //[FreeFunction("QuaternionScripting::Lerp", IsThreadSafe = true)]
    public static Quaternion Lerp(Quaternion a, Quaternion b, float t)
    {
        Lerp_Injected(ref a, ref b, t, out var ret);
        return ret;
    }

    //
    // Summary:
    //     Interpolates between a and b by t and normalizes the result afterwards. The parameter
    //     t is not clamped.
    //
    // Parameters:
    //   a:
    //
    //   b:
    //
    //   t:
    //[FreeFunction("QuaternionScripting::LerpUnclamped", IsThreadSafe = true)]
    public static Quaternion LerpUnclamped(Quaternion a, Quaternion b, float t)
    {
        LerpUnclamped_Injected(ref a, ref b, t, out var ret);
        return ret;
    }

    //[FreeFunction("EulerToQuaternion", IsThreadSafe = true)]
    private static Quaternion Internal_FromEulerRad(Vector3 euler)
    {
        Internal_FromEulerRad_Injected(ref euler, out var ret);
        return ret;
    }

    //[FreeFunction("QuaternionScripting::ToEuler", IsThreadSafe = true)]
    private static Vector3 Internal_ToEulerRad(Quaternion rotation)
    {
        Internal_ToEulerRad_Injected(ref rotation, out var ret);
        return ret;
    }

    //[FreeFunction("QuaternionScripting::ToAxisAngle", IsThreadSafe = true)]
    private static void Internal_ToAxisAngleRad(Quaternion q, out Vector3 axis, out float angle)
    {
        Internal_ToAxisAngleRad_Injected(ref q, out axis, out angle);
    }

    //
    // Summary:
    //     Creates a rotation which rotates angle degrees around axis.
    //
    // Parameters:
    //   angle:
    //
    //   axis:
    //[FreeFunction("QuaternionScripting::AngleAxis", IsThreadSafe = true)]
    public static Quaternion AngleAxis(float angle, Vector3 axis)
    {
        AngleAxis_Injected(angle, ref axis, out var ret);
        return ret;
    }

    //
    // Summary:
    //     Creates a rotation with the specified forward and upwards directions.
    //
    // Parameters:
    //   forward:
    //     The direction to look in.
    //
    //   upwards:
    //     The vector that defines in which direction up is.
    //[FreeFunction("QuaternionScripting::LookRotation", IsThreadSafe = true)]
    public static Quaternion LookRotation(Vector3 forward, Vector3 upwards)
    {
        LookRotation_Injected(ref forward, ref upwards, out var ret);
        return ret;
    }

    //
    // Summary:
    //     Creates a rotation with the specified forward and upwards directions.
    //
    // Parameters:
    //   forward:
    //     The direction to look in.
    //
    //   upwards:
    //     The vector that defines in which direction up is.
    //[ExcludeFromDocs]
    public static Quaternion LookRotation(Vector3 forward)
    {
        return LookRotation(forward, Vector3.up);
    }

    //
    // Summary:
    //     Constructs new Quaternion with given x,y,z,w components.
    //
    // Parameters:
    //   x:
    //
    //   y:
    //
    //   z:
    //
    //   w:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public Quaternion(float x, float y, float z, float w)
    {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }

    //
    // Summary:
    //     Set x, y, z and w components of an existing Quaternion.
    //
    // Parameters:
    //   newX:
    //
    //   newY:
    //
    //   newZ:
    //
    //   newW:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public void Set(float newX, float newY, float newZ, float newW)
    {
        x = newX;
        y = newY;
        z = newZ;
        w = newW;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Quaternion operator *(Quaternion lhs, Quaternion rhs)
    {
        return new Quaternion(lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y, lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z, lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x, lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z);
    }

    public static Vector3 operator *(Quaternion rotation, Vector3 point)
    {
        float num = rotation.x * 2f;
        float num2 = rotation.y * 2f;
        float num3 = rotation.z * 2f;
        float num4 = rotation.x * num;
        float num5 = rotation.y * num2;
        float num6 = rotation.z * num3;
        float num7 = rotation.x * num2;
        float num8 = rotation.x * num3;
        float num9 = rotation.y * num3;
        float num10 = rotation.w * num;
        float num11 = rotation.w * num2;
        float num12 = rotation.w * num3;
        Vector3 result = default(Vector3);
        result.x = (1f - (num5 + num6)) * point.x + (num7 - num12) * point.y + (num8 + num11) * point.z;
        result.y = (num7 + num12) * point.x + (1f - (num4 + num6)) * point.y + (num9 - num10) * point.z;
        result.z = (num8 - num11) * point.x + (num9 + num10) * point.y + (1f - (num4 + num5)) * point.z;
        return result;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static bool IsEqualUsingDot(float dot)
    {
        return dot > 0.999999f;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Quaternion lhs, Quaternion rhs)
    {
        return IsEqualUsingDot(Dot(lhs, rhs));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Quaternion lhs, Quaternion rhs)
    {
        return !(lhs == rhs);
    }

    //
    // Summary:
    //     The dot product between two rotations.
    //
    // Parameters:
    //   a:
    //
    //   b:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float Dot(Quaternion a, Quaternion b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    //
    // Summary:
    //     Creates a rotation with the specified forward and upwards directions.
    //
    // Parameters:
    //   view:
    //     The direction to look in.
    //
    //   up:
    //     The vector that defines in which direction up is.
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    //[ExcludeFromDocs]
    public void SetLookRotation(Vector3 view)
    {
        Vector3 up = Vector3.up;
        SetLookRotation(view, up);
    }

    //
    // Summary:
    //     Creates a rotation with the specified forward and upwards directions.
    //
    // Parameters:
    //   view:
    //     The direction to look in.
    //
    //   up:
    //     The vector that defines in which direction up is.
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public void SetLookRotation(Vector3 view, Vector3 up)
    {
        this = LookRotation(view, up);
    }

    //
    // Summary:
    //     Returns the angle in degrees between two rotations a and b.
    //
    // Parameters:
    //   a:
    //
    //   b:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float Angle(Quaternion a, Quaternion b)
    {
        float num = Mathf.Min(Mathf.Abs(Dot(a, b)), 1f);
        return IsEqualUsingDot(num) ? 0f : (Mathf.Acos(num) * 2f * 57.29578f);
    }

    private static Vector3 Internal_MakePositive(Vector3 euler)
    {
        float num = -0.005729578f;
        float num2 = 360f + num;
        if (euler.x < num)
        {
            euler.x += 360f;
        }
        else if (euler.x > num2)
        {
            euler.x -= 360f;
        }

        if (euler.y < num)
        {
            euler.y += 360f;
        }
        else if (euler.y > num2)
        {
            euler.y -= 360f;
        }

        if (euler.z < num)
        {
            euler.z += 360f;
        }
        else if (euler.z > num2)
        {
            euler.z -= 360f;
        }

        return euler;
    }

    //
    // Summary:
    //     Returns a rotation that rotates z degrees around the z axis, x degrees around
    //     the x axis, and y degrees around the y axis; applied in that order.
    //
    // Parameters:
    //   x:
    //
    //   y:
    //
    //   z:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Quaternion Euler(float x, float y, float z)
    {
        return Internal_FromEulerRad(new Vector3(x, y, z) * (MathF.PI / 180f));
    }

    //
    // Summary:
    //     Returns a rotation that rotates z degrees around the z axis, x degrees around
    //     the x axis, and y degrees around the y axis.
    //
    // Parameters:
    //   euler:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Quaternion Euler(Vector3 euler)
    {
        return Internal_FromEulerRad(euler * (MathF.PI / 180f));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public void ToAngleAxis(out float angle, out Vector3 axis)
    {
        Internal_ToAxisAngleRad(this, out axis, out angle);
        angle *= 57.29578f;
    }

    //
    // Summary:
    //     Creates a rotation which rotates from fromDirection to toDirection.
    //
    // Parameters:
    //   fromDirection:
    //
    //   toDirection:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public void SetFromToRotation(Vector3 fromDirection, Vector3 toDirection)
    {
        this = FromToRotation(fromDirection, toDirection);
    }

    //
    // Summary:
    //     Rotates a rotation from towards to.
    //
    // Parameters:
    //   from:
    //
    //   to:
    //
    //   maxDegreesDelta:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Quaternion RotateTowards(Quaternion from, Quaternion to, float maxDegreesDelta)
    {
        float num = Angle(from, to);
        if (num == 0f)
        {
            return to;
        }

        return SlerpUnclamped(from, to, Mathf.Min(1f, maxDegreesDelta / num));
    }

    //
    // Summary:
    //     Converts this quaternion to one with the same orientation but with a magnitude
    //     of 1.
    //
    // Parameters:
    //   q:
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Quaternion Normalize(Quaternion q)
    {
        float num = Mathf.Sqrt(Dot(q, q));
        if (num < Mathf.Epsilon)
        {
            return identity;
        }

        return new Quaternion(q.x / num, q.y / num, q.z / num, q.w / num);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public void Normalize()
    {
        this = Normalize(this);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override int GetHashCode()
    {
        return x.GetHashCode() ^ (y.GetHashCode() << 2) ^ (z.GetHashCode() >> 2) ^ (w.GetHashCode() >> 1);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override bool Equals(object other)
    {
        if (!(other is Quaternion))
        {
            return false;
        }

        return Equals((Quaternion)other);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public bool Equals(Quaternion other)
    {
        return x.Equals(other.x) && y.Equals(other.y) && z.Equals(other.z) && w.Equals(other.w);
    }

    //
    // Summary:
    //     Returns a formatted string for this quaternion.
    //
    // Parameters:
    //   format:
    //     A numeric format string.
    //
    //   formatProvider:
    //     An object that specifies culture-specific formatting.
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public override string ToString()
    {
        return ToString(null, null);
    }

    //
    // Summary:
    //     Returns a formatted string for this quaternion.
    //
    // Parameters:
    //   format:
    //     A numeric format string.
    //
    //   formatProvider:
    //     An object that specifies culture-specific formatting.
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public string ToString(string format)
    {
        return ToString(format, null);
    }

    //
    // Summary:
    //     Returns a formatted string for this quaternion.
    //
    // Parameters:
    //   format:
    //     A numeric format string.
    //
    //   formatProvider:
    //     An object that specifies culture-specific formatting.
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public string ToString(string format, IFormatProvider formatProvider)
    {
        if (string.IsNullOrEmpty(format))
        {
            format = "F5";
        }

        if (formatProvider == null)
        {
            formatProvider = CultureInfo.InvariantCulture.NumberFormat;
        }

        return string.Format("({0}, {1}, {2}, {3})", x.ToString(format, formatProvider), y.ToString(format, formatProvider), z.ToString(format, formatProvider), w.ToString(format, formatProvider));
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void FromToRotation_Injected(ref Vector3 fromDirection, ref Vector3 toDirection, out Quaternion ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Inverse_Injected(ref Quaternion rotation, out Quaternion ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Slerp_Injected(ref Quaternion a, ref Quaternion b, float t, out Quaternion ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void SlerpUnclamped_Injected(ref Quaternion a, ref Quaternion b, float t, out Quaternion ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Lerp_Injected(ref Quaternion a, ref Quaternion b, float t, out Quaternion ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void LerpUnclamped_Injected(ref Quaternion a, ref Quaternion b, float t, out Quaternion ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_FromEulerRad_Injected(ref Vector3 euler, out Quaternion ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_ToEulerRad_Injected(ref Quaternion rotation, out Vector3 ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_ToAxisAngleRad_Injected(ref Quaternion q, out Vector3 axis, out float angle);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void AngleAxis_Injected(float angle, ref Vector3 axis, out Quaternion ret);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void LookRotation_Injected(ref Vector3 forward, ref Vector3 upwards, out Quaternion ret);

}

