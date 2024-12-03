using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public class Scene : Asset<Scene>
{
	public static void LoadScene(string key)
	{
		internal_m2n_load_scene(key);
	}

	public static Entity Instantiate(Prefab prefab)
	{
		return internal_m2n_create_entity_from_prefab_uid(prefab.uid);;
	}

	public static Entity Instantiate(string key)
	{
		return internal_m2n_create_entity_from_prefab_key(key);
	}

	public static Entity CloneEntity(Entity e)
	{
		return internal_m2n_clone_entity(e);
	}
	
	public static Entity CreateEntity(string tag = "Unnamed")
	{
		return internal_m2n_create_entity(tag);
	}

	public static void DestroyEntity(Entity entity)
	{
		unsafe
		{
			float seconds = 0.0f;
			internal_m2n_destroy_entity(entity, seconds);
		}
	}

	public static void DestroyEntity(Entity entity, float seconds)
	{
		unsafe
		{
			internal_m2n_destroy_entity(entity, seconds);
		}
	}

	public static void DestroyEntityImmediate(Entity entity)
	{
		unsafe
		{
			internal_m2n_destroy_entity_immediate(entity);
		}
	}

	public static bool IsEntityValid(Entity entity)
	{
		return internal_m2n_is_entity_valid(entity);
	}
	
	public static Entity FindEntityByTag(string tag = "Unnamed")
	{
		return internal_m2n_find_entity_by_tag(tag);
	}

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern void internal_m2n_load_scene(string key);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private extern void internal_m2n_create_scene();

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private extern void internal_m2n_destroy_scene();

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Entity internal_m2n_create_entity_from_prefab_uid(Guid uid);
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Entity internal_m2n_create_entity_from_prefab_key(string key);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Entity internal_m2n_create_entity(string tag);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Entity internal_m2n_clone_entity(Entity id);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_destroy_entity(Entity id, float seconds);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_destroy_entity_immediate(Entity id);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_is_entity_valid(Entity id);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern Entity internal_m2n_find_entity_by_tag(string tag);
	
}

}
}


