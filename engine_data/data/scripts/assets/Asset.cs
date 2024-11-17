using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{
public class Asset<T> : IEquatable<Asset<T>>
{
    
    public Guid uid {get; internal set;}

	public override bool Equals(object obj)
    {
        if (obj == null || !(obj is Asset<T>))
            return false;

        return Equals((Asset<T>)obj);
    }

	public bool Equals(Asset<T> other)
	{
        if(other == null)
            return false;

		if (ReferenceEquals(this, other))
			return true;

		return this.uid == other.uid;
	}


	public static bool operator ==(Asset<T> lhs, Asset<T> rhs) => lhs.Equals(rhs);

	public static bool operator !=(Asset<T> lhs, Asset<T> rhs) => !(lhs == rhs);
	
    
	public override int GetHashCode() => (uid).GetHashCode();
}


}

}


