using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public class NativeObject : IEquatable<NativeObject>
{
    protected bool m_alive = false;

    // Implement IEquatable<NativeObject>
    public bool Equals(NativeObject other)
    {
        // Check for null and compare run-time types.
        if (ReferenceEquals(other, null))
            return false;

        if (ReferenceEquals(this, other))
            return true;

        // If either object is not alive, they are not equal.
        if (!m_alive || !other.m_alive)
            return false;

        // Add additional field comparisons here if needed.
        // For example, if you have an 'id' field:
        // return this.id == other.id;

        // If there's no additional state to compare, and 'm_alive' is true, 
        // you might consider them equal only if they are the same instance.
        return false;
    }

    // Override Equals(object)
    public override bool Equals(object obj)
    {
        if (ReferenceEquals(obj, null))
            return false;

        if (!m_alive)
            return false;

        if (ReferenceEquals(this, obj))
            return true;

        // Ensure the object is of the same type.
        if (obj.GetType() != this.GetType())
            return false;

        return Equals(obj as NativeObject);
    }

    // Override GetHashCode()
    public override int GetHashCode()
    {
        if (!m_alive)
            return 0; // Or some constant to represent 'not alive' state.

        // Include 'm_alive' in the hash code if it's part of equality.
        // If you have other fields, include them in the hash code.
        // For example:
        // int hash = 17;
        // hash = hash * 23 + m_alive.GetHashCode();
        // hash = hash * 23 + id.GetHashCode();
        // return hash;

        // If no fields to include, you can use base.GetHashCode() or RuntimeHelpers.
        return base.GetHashCode();
    }

    // Overload operator ==
    public static bool operator ==(NativeObject left, NativeObject right)
    {
        if (ReferenceEquals(left, right))
            return true;

        if (ReferenceEquals(left, null) || !left.m_alive)
            return ReferenceEquals(right, null) || !right.m_alive;

        if (ReferenceEquals(right, null) || !right.m_alive)
            return false;

        return left.Equals(right);
    }

    // Overload operator !=
    public static bool operator !=(NativeObject left, NativeObject right)
    {
        return !(left == right);
    }
}

}

}


