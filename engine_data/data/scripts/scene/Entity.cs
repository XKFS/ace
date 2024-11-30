using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{
[StructLayout(LayoutKind.Sequential)]
public struct Entity : IEquatable<Entity>
{
	public readonly uint Id;

	public string tag
    {
        get
        {
            return internal_m2n_get_tag(this);
        }
		set
        {
            internal_m2n_set_tag(this, value);
        }
    }

	public TransformComponent transform
    {
        get
        {
            return internal_m2n_get_transform_component(this, typeof(TransformComponent)) as TransformComponent;
        }
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
		return internal_m2n_add_component(this, typeof(T)) as T; 
	}

	public bool HasComponent<T>() where T : Component
	{
		return internal_m2n_has_component(this, typeof(T)); 
	}

	public bool HasComponent(Type type) 
	{
		return internal_m2n_has_component(this, type); 
	}

	public T GetComponent<T>() where T : Component, new()
	{
		return internal_m2n_get_component(this, typeof(T)) as T; 
	}

	    //
    // Summary:
    //     The non-generic version of this method.
    //
    // Parameters:
    //   type:
    //     The type of component to search for.
    //
    // Returns:
    //     An array containing all matching components of type type.
    public Component[] GetComponents(Type type)
    {
        return (Component[])internal_m2n_get_components(this, type);
    }

    public T[] GetComponents<T>() where T : Component
    {
		return (T[])internal_m2n_get_components(this, typeof(T));

    }
	public bool RemoveComponent(Component component)
	{
		return internal_m2n_remove_component_instance(this, component);
	}

	public bool RemoveComponent<T>() where T : Component
	{
		return internal_m2n_remove_component(this, typeof(T)); 
	}

	internal Entity(uint id)
	{
		Id = id;
	}

		
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern string internal_m2n_get_tag(Entity id);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern void internal_m2n_set_tag(Entity id, string tag);


	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Component internal_m2n_add_component(Entity id, Type obj);
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Component internal_m2n_get_component(Entity id, Type obj);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Array internal_m2n_get_components(Entity id, Type obj);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_has_component(Entity id, Type obj);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_remove_component_instance(Entity id, Component obj);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_remove_component(Entity id, Type obj);


	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Component internal_m2n_get_transform_component(Entity id, Type obj);



}
}
}


