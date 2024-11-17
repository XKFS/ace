using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public class Scene
{

	public Scene()
	{
		internal_m2n_create_scene();
	}

	~Scene()
	{
		internal_m2n_destroy_scene();
	}

	public static void LoadScene(string key)
	{
		internal_m2n_load_scene(key);
	}

	public static Entity Instantiate(Prefab prefab)
	{
		unsafe { return new Entity(internal_m2n_create_entity_from_prefab_uid(prefab.uid)); }
	}

	public static Entity Instantiate(string key)
	{
		unsafe { return new Entity(internal_m2n_create_entity_from_prefab_key(key)); }
	}

	public static Entity CloneEntity(Entity e)
	{
		unsafe { return new Entity(internal_m2n_clone_entity(e.Id)); }
	}
	
	public static Entity CreateEntity(string tag = "Unnamed")
	{
		unsafe { return new Entity(internal_m2n_create_entity(tag)); }
	}

	public static void DestroyEntity(Entity entity)
	{
		unsafe
		{
			internal_m2n_destroy_entity(entity.Id);
		}
	}

	public static bool IsEntityValid(Entity entity)
	{
		return internal_m2n_is_entity_valid(entity.Id);
	}
	
	public static Entity FindEntityByTag(string tag = "Unnamed")
	{
		unsafe { return new Entity(internal_m2n_find_entity_by_tag(tag)); }
	}

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern void internal_m2n_load_scene(string key);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private extern void internal_m2n_create_scene();

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private extern void internal_m2n_destroy_scene();

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern uint internal_m2n_create_entity_from_prefab_uid(Guid uid);
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern uint internal_m2n_create_entity_from_prefab_key(string key);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern uint internal_m2n_create_entity(string tag);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern uint internal_m2n_clone_entity(uint id);
	
	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_destroy_entity(uint id);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern bool internal_m2n_is_entity_valid(uint id);

	[MethodImpl(MethodImplOptions.InternalCall)] 
	private static extern uint internal_m2n_find_entity_by_tag(string tag);
	
}

}
}


