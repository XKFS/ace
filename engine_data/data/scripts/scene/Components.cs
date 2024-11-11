using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public abstract class Component : NativeObject
{
    public override bool IsValid()
    {
        if(!owner.IsValid())
        {
            return false;
        }
        return owner.HasComponent(GetType());
    }
    public Entity owner { get; internal set; }
    private void internal_n2m_set_entity(uint id)
    {
        owner = new Entity(id);
    }
}

public class TransformComponent : Component
{
    //
    // Summary:
    //     The world space position of the Transform.
    public Vector3 positionGlobal
    {
        get
        {
            return internal_m2n_get_position_global(owner.Id);
        }
        set
        {
            internal_m2n_set_position_global(owner.Id, ref value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Vector3 positionLocal
    {
        get
        {
            return internal_m2n_get_position_local(owner.Id);
        }
        set
        {
            internal_m2n_set_position_local(owner.Id, ref value);
        }
    }


    public Vector3 rotationEulerGlobal
    {
        get
        {
            return internal_m2n_get_rotation_euler_global(owner.Id);
        }
        set
        {
            internal_m2n_set_rotation_euler_global(owner.Id, ref value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Vector3 rotationEulerLocal
    {
        get
        {
            return internal_m2n_get_rotation_euler_local(owner.Id);
        }
        set
        {
            internal_m2n_set_rotation_euler_local(owner.Id, ref value);
        }
    }


    public Quaternion rotationGlobal
    {
        get
        {
            return internal_m2n_get_rotation_global(owner.Id);
        }
        set
        {
            internal_m2n_set_rotation_global(owner.Id, ref value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Quaternion rotationLocal
    {
        get
        {
            return internal_m2n_get_rotation_local(owner.Id);
        }
        set
        {
            internal_m2n_set_rotation_local(owner.Id, ref value);
        }
    }

    public Vector3 scaleGlobal
    {
        get
        {
            return internal_m2n_get_scale_global(owner.Id);
        }
        set
        {
            internal_m2n_set_scale_global(owner.Id, ref value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Vector3 scaleLocal
    {
        get
        {
            return internal_m2n_get_scale_local(owner.Id);
        }
        set
        {
            internal_m2n_set_scale_local(owner.Id, ref value);
        }
    }

    public Vector3 skewGlobal
    {
        get
        {
            return internal_m2n_get_skew_global(owner.Id);
        }
        set
        {
            internal_m2n_set_skew_global(owner.Id, ref value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Vector3 skewLocal
    {
        get
        {
            return internal_m2n_get_skew_local(owner.Id);
        }
        set
        {
            internal_m2n_set_skew_local(owner.Id, ref value);
        }
    }


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_position_global(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_position_global(uint eid, ref Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_position_local(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_position_local(uint eid, ref Vector3 value);



    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_rotation_euler_global(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_rotation_euler_global(uint eid, ref Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_rotation_euler_local(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_rotation_euler_local(uint eid, ref Vector3 value);

    
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Quaternion internal_m2n_get_rotation_global(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_rotation_global(uint eid, ref Quaternion value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Quaternion internal_m2n_get_rotation_local(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_rotation_local(uint eid, ref Quaternion value);



    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_scale_global(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_scale_global(uint eid, ref Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_scale_local(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_scale_local(uint eid, ref Vector3 value);



    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_skew_global(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_skew_global(uint eid, ref Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_skew_local(uint eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_skew_local(uint eid, ref Vector3 value);
}

public abstract class ScriptComponent : Component
{

    public ScriptComponent()
    {
        SystemManager.OnUpdate += OnUpdate;
    }

    ~ScriptComponent()
    {
        SystemManager.OnUpdate -= OnUpdate;
    }

    public virtual void OnCreate()
    {

    }
    public virtual void OnStart()
    {

    }

    public virtual void OnDestroy()
    {

    }

    public virtual void OnUpdate()
    {

    }


    private void internal_n2m_on_create()
    {
        OnCreate();
    }
    private void internal_n2m_on_start()
    {
        OnStart();
    }

    private void internal_n2m_on_destroy()
    {
        OnDestroy();
    }
}

}
}


