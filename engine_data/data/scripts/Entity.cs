using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{

public struct Entity : IEquatable<Entity>
{
	public readonly uint Id;
	
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

		return internal_m2n_add_component(Id, typeof(T)) as T; 
	}

	public bool HasComponent<T>() where T : Component
	{
		if(!Scene.IsEntityValid(this))
		{
			throw new SystemException("Entity is Invalid.");
		}
		return internal_m2n_has_component(Id, typeof(T)); 
	}

	public bool HasComponent(Type type) 
	{
		if(!Scene.IsEntityValid(this))
		{
			throw new SystemException("Entity is Invalid.");
		}
		return internal_m2n_has_component(Id, type); 
	}

	public T GetComponent<T>() where T : Component, new()
	{
		if(!Scene.IsEntityValid(this))
		{
			throw new SystemException("Entity is Invalid.");
		}
		return internal_m2n_get_component(Id, typeof(T)) as T; 
	}

	public bool RemoveComponent(Component component)
	{
		if(!Scene.IsEntityValid(this))
		{
			throw new SystemException("Entity is Invalid.");
		}
		return internal_m2n_remove_component(Id, component);
	}

	internal Entity(uint id)
	{
		Id = id;
	}
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Component internal_m2n_add_component(uint id, Type obj);
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Component internal_m2n_get_component(uint id, Type obj);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_has_component(uint id, Type obj);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_remove_component(uint id, Component obj);



}
}
}


