using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{

public class TransformComponent : Component
{
    //
    // Summary:
    //     The world space position of the Transform.
    public Vector3 positionGlobal
    {
        get
        {
            return internal_m2n_get_position_global(owner);
        }
        set
        {
            internal_m2n_set_position_global(owner, value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Vector3 positionLocal
    {
        get
        {
            return internal_m2n_get_position_local(owner);
        }
        set
        {
            internal_m2n_set_position_local(owner, value);
        }
    }


    public Vector3 rotationEulerGlobal
    {
        get
        {
            return internal_m2n_get_rotation_euler_global(owner);
        }
        set
        {
            internal_m2n_set_rotation_euler_global(owner, value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Vector3 rotationEulerLocal
    {
        get
        {
            return internal_m2n_get_rotation_euler_local(owner);
        }
        set
        {
            internal_m2n_set_rotation_euler_local(owner, value);
        }
    }


    public Quaternion rotationGlobal
    {
        get
        {
            return internal_m2n_get_rotation_global(owner);
        }
        set
        {
            internal_m2n_set_rotation_global(owner, value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Quaternion rotationLocal
    {
        get
        {
            return internal_m2n_get_rotation_local(owner);
        }
        set
        {
            internal_m2n_set_rotation_local(owner, value);
        }
    }

    public Vector3 scaleGlobal
    {
        get
        {
            return internal_m2n_get_scale_global(owner);
        }
        set
        {
            internal_m2n_set_scale_global(owner, value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Vector3 scaleLocal
    {
        get
        {
            return internal_m2n_get_scale_local(owner);
        }
        set
        {
            internal_m2n_set_scale_local(owner, value);
        }
    }

    public Vector3 skewGlobal
    {
        get
        {
            return internal_m2n_get_skew_global(owner);
        }
        set
        {
            internal_m2n_set_skew_global(owner, value);
        }
    }

    //
    // Summary:
    //     Position of the transform relative to the parent transform.
    public Vector3 skewLocal
    {
        get
        {
            return internal_m2n_get_skew_local(owner);
        }
        set
        {
            internal_m2n_set_skew_local(owner, value);
        }
    }

    public void MoveByGlobal(Vector3 amount)
    {
        internal_m2n_move_by_global(owner, amount);
    }

    public void MoveByLocal(Vector3 amount)
    {
        internal_m2n_move_by_local(owner, amount);
    }
    
    public void ScaleByGlobal(Vector3 amount)
    {
        internal_m2n_scale_by_global(owner, amount);
    }

    public void ScaleByLocal(Vector3 amount)
    {
        internal_m2n_scale_by_local(owner, amount);
    }

    public void RotateByGlobal(Quaternion amount)
    {
        internal_m2n_rotate_by_global(owner, amount);
    }

    public void RotateByLocal(Quaternion amount)
    {
        internal_m2n_rotate_by_local(owner, amount);
    }

    public void RotateByEulerGlobal(Vector3 amount)
    {
        internal_m2n_rotate_by_euler_global(owner, amount);
    }

    public void RotateByEulerLocal(Vector3 amount)
    {
        internal_m2n_rotate_by_euler_local(owner, amount);
    }

    public void RotateAxisGlobal(float degrees, Vector3 axis)
    {
        internal_m2n_rotate_axis_global(owner, degrees, axis);
    }
    public void LookAt(Vector3 point)
    {
        LookAt(point, Vector3.up);
    }

    public void LookAt(Vector3 point, Vector3 up)
    {
        internal_m2n_look_at(owner, point, up);
    }

    public void LookAt(Entity target)
    {
        LookAt(target, Vector3.up);
    }
    
    public void LookAt(Entity target, Vector3 up)
    {
        var transform = target.GetComponent<TransformComponent>();
        LookAt(transform.positionGlobal, up);
    }

    public void RotateAroundGlobal(Vector3 point, Vector3 axis, float angle)
    {
        Vector3 vector = positionGlobal;
        Quaternion quaternion = Quaternion.AngleAxis(angle, axis);
        Vector3 vector2 = vector - point;
        vector2 = quaternion * vector2;
        vector = point + vector2;
        positionGlobal = vector;
        RotateAxisGlobal(angle, axis);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_position_global(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_position_global(Entity eid, Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_move_by_global(Entity eid, Vector3 amount);


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_position_local(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_position_local(Entity eid, Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_move_by_local(Entity eid, Vector3 amount);



    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_rotation_euler_global(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_rotation_euler_global(Entity eid, Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_rotate_by_euler_global(Entity eid, Vector3 amount);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_rotate_axis_global(Entity eid, float degrees, Vector3 axis);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_look_at(Entity eid, Vector3 point, Vector3 up);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_rotation_euler_local(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_rotation_euler_local(Entity eid, Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_rotate_by_euler_local(Entity eid, Vector3 amount);


    
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Quaternion internal_m2n_get_rotation_global(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_rotation_global(Entity eid, Quaternion value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_rotate_by_global(Entity eid, Quaternion amount);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Quaternion internal_m2n_get_rotation_local(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_rotation_local(Entity eid, Quaternion value);
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_rotate_by_local(Entity eid, Quaternion amount);


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_scale_global(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_scale_global(Entity eid, Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_scale_by_global(Entity eid, Vector3 amount);


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_scale_local(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_scale_local(Entity eid, Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_scale_by_local(Entity eid, Vector3 amount);


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_skew_global(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_skew_global(Entity eid, Vector3 value);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Vector3 internal_m2n_get_skew_local(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_set_skew_local(Entity eid, Vector3 value);

}

}
}


