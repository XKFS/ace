using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{

public struct Entity : IEquatable<Entity>
{
	public readonly uint Id;
	
	internal Entity(uint id)
	{
		Id = id;
	}
	
	public override bool Equals(object obj)
    {
        if (obj == null || !(obj is Entity))
            return false;

        return Equals((Entity)obj);
    }

	public bool Equals(Entity other)
	{
		if (ReferenceEquals(this, other))
			return true;

		return Id == other.Id;
	}
	
	public static bool operator ==(Entity lhs, Entity rhs) => lhs.Equals(rhs);

	public static bool operator !=(Entity lhs, Entity rhs) => !(lhs == rhs);
	
	public override int GetHashCode() => (Id).GetHashCode();

	public bool IsValid()
	{
		return Scene.IsEntityValid(this);
	}

	public T AddComponent<T>() where T : Component, new()
	{
		if(!Scene.IsEntityValid(this))
		{
			throw new SystemException("Entity is Invalid.");
		}

		return Internal_AddComponent(Id, typeof(T)) as T; 
	}

	public T GetComponent<T>() where T : Component, new()
	{
		if(!Scene.IsEntityValid(this))
		{
			throw new SystemException("Entity is Invalid.");
		}
		return Internal_GetComponent(Id, typeof(T)) as T; 
	}

	public bool RemoveComponent(Component component)
	{
		if(!Scene.IsEntityValid(this))
		{
			throw new SystemException("Entity is Invalid.");
		}
		return Internal_RemoveComponent(Id, component);
	}
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Component Internal_AddComponent(uint id, Type obj);
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Component Internal_GetComponent(uint id, Type obj);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool Internal_RemoveComponent(uint id, Component obj);



}
}
}


