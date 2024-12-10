using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{
/// <summary>
/// Represents an entity within a scene. Provides methods to manage components and query entity properties.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Entity : IEquatable<Entity>
{
	/// <summary>
	/// Gets the unique identifier of the entity.
	/// </summary>
	public readonly uint Id;

	/// <summary>
	/// Gets or sets the name of the entity.
	/// </summary>
	/// <value>The name associated with the entity.</value>
	public string name
	{
		get => internal_m2n_get_name(this);
		set => internal_m2n_set_name(this, value);
	}

	/// <summary>
	/// Gets or sets the tag of the entity.
	/// </summary>
	/// <value>The tag associated with the entity.</value>
	public string tag
	{
		get => internal_m2n_get_tag(this);
		set => internal_m2n_set_tag(this, value);
	}

	/// <summary>
	/// Gets the transform component of the entity.
	/// </summary>
	/// <value>The transform component of the entity.</value>
	public TransformComponent transform
	{
		get => internal_m2n_get_transform_component(this, typeof(TransformComponent)) as TransformComponent;
	}

	/// <inheritdoc/>
	public override bool Equals(object obj)
	{
		if (obj == null || !(obj is Entity))
			return false;

		return Equals((Entity)obj);
	}

	/// <summary>
	/// Determines whether the specified entity is equal to the current entity.
	/// </summary>
	/// <param name="other">The entity to compare with the current entity.</param>
	/// <returns><c>true</c> if the specified entity is equal to the current entity; otherwise, <c>false</c>.</returns>
	public bool Equals(Entity other)
	{
		if (ReferenceEquals(this, other))
			return true;

		return Id == other.Id;
	}

	/// <summary>
	/// Compares two entities for equality.
	/// </summary>
	/// <param name="lhs">The first entity.</param>
	/// <param name="rhs">The second entity.</param>
	/// <returns><c>true</c> if both entities are equal; otherwise, <c>false</c>.</returns>
	public static bool operator ==(Entity lhs, Entity rhs) => lhs.Equals(rhs);

	/// <summary>
	/// Compares two entities for inequality.
	/// </summary>
	/// <param name="lhs">The first entity.</param>
	/// <param name="rhs">The second entity.</param>
	/// <returns><c>true</c> if both entities are not equal; otherwise, <c>false</c>.</returns>
	public static bool operator !=(Entity lhs, Entity rhs) => !(lhs == rhs);

	/// <inheritdoc/>
	public override int GetHashCode() => Id.GetHashCode();

	/// <summary>
	/// Determines whether the entity is valid within the scene.
	/// </summary>
	/// <returns><c>true</c> if the entity is valid; otherwise, <c>false</c>.</returns>
	public bool IsValid()
	{
		return Scene.IsEntityValid(this);
	}

	/// <summary>
	/// Adds a new component of the specified type to the entity.
	/// </summary>
	/// <typeparam name="T">The type of component to add.</typeparam>
	/// <returns>The newly added component.</returns>
	public T AddComponent<T>() where T : Component, new()
	{
		return internal_m2n_add_component(this, typeof(T)) as T;
	}

	/// <summary>
	/// Determines whether the entity has a component of the specified type.
	/// </summary>
	/// <typeparam name="T">The type of component to check for.</typeparam>
	/// <returns><c>true</c> if the entity has the specified component; otherwise, <c>false</c>.</returns>
	public bool HasComponent<T>() where T : Component
	{
		return internal_m2n_has_component(this, typeof(T));
	}

	/// <summary>
	/// Determines whether the entity has a component of the specified type.
	/// </summary>
	/// <param name="type">The type of component to check for.</param>
	/// <returns><c>true</c> if the entity has the specified component; otherwise, <c>false</c>.</returns>
	public bool HasComponent(Type type)
	{
		return internal_m2n_has_component(this, type);
	}

	/// <summary>
	/// Gets the component of the specified type from the entity.
	/// </summary>
	/// <typeparam name="T">The type of component to retrieve.</typeparam>
	/// <returns>The component of the specified type.</returns>
	public T GetComponent<T>() where T : Component, new()
	{
		return internal_m2n_get_component(this, typeof(T)) as T;
	}

	/// <summary>
	/// Gets all components of the specified type from the entity.
	/// </summary>
	/// <param name="type">The type of component to retrieve.</param>
	/// <returns>An array of components of the specified type.</returns>
	public Component[] GetComponents(Type type)
	{
		return (Component[])internal_m2n_get_components(this, type);
	}

	/// <summary>
	/// Gets all components of the specified type from the entity.
	/// </summary>
	/// <typeparam name="T">The type of component to retrieve.</typeparam>
	/// <returns>An array of components of the specified type.</returns>
	public T[] GetComponents<T>() where T : Component
	{
		return (T[])internal_m2n_get_components(this, typeof(T));
	}

	/// <summary>
	/// Removes a specified component instance from the entity.
	/// </summary>
	/// <param name="component">The component instance to remove.</param>
	/// <returns><c>true</c> if the component was successfully removed; otherwise, <c>false</c>.</returns>
	public bool RemoveComponent(Component component)
	{
		return internal_m2n_remove_component_instance(this, component);
	}

	/// <summary>
	/// Removes a component of the specified type from the entity.
	/// </summary>
	/// <typeparam name="T">The type of component to remove.</typeparam>
	/// <returns><c>true</c> if the component was successfully removed; otherwise, <c>false</c>.</returns>
	public bool RemoveComponent<T>() where T : Component
	{
		return internal_m2n_remove_component(this, typeof(T));
	}

	/// <summary>
	/// Initializes a new instance of the <see cref="Entity"/> struct with the specified identifier.
	/// </summary>
	/// <param name="id">The unique identifier of the entity.</param>
	internal Entity(uint id)
	{
		Id = id;
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern string internal_m2n_get_name(Entity id);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void internal_m2n_set_name(Entity id, string name);

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
