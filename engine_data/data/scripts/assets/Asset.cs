using System;

namespace Ace
{
namespace Core
{
public class Asset<T> : IEquatable<Asset<T>>
{
	public Guid uid { get; internal set; }

	public override bool Equals(object obj)
	{
		// Check for null and type compatibility
		if (obj is Asset<T> other)
			return Equals(other);

		return false;
	}

	public bool Equals(Asset<T> other)
	{
		// Check for null and reference equality
		if (ReferenceEquals(other, null))
			return false;

		if (ReferenceEquals(this, other))
			return true;

		// Compare UIDs
		return this.uid == other.uid;
	}

	public static bool operator ==(Asset<T> lhs, Asset<T> rhs)
	{
		// Handle null references safely
		if (ReferenceEquals(lhs, rhs))
			return true;

		if (ReferenceEquals(lhs, null) || ReferenceEquals(rhs, null))
			return false;

		return lhs.Equals(rhs);
	}

	public static bool operator !=(Asset<T> lhs, Asset<T> rhs) => !(lhs == rhs);

	public override int GetHashCode() => uid.GetHashCode();
}
}
}
