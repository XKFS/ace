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
    public Vector3 position
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
    public Vector3 localPosition
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


    public Vector3 eulerAngles
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

    public Vector3 localEulerAngles
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


    public Quaternion rotation
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
    public Quaternion localRotation
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

    public Vector3 scale
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
    public Vector3 localScale
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

    public Vector3 skew
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

    public Vector3 localSkew
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

    
    public Vector3 right
    {
        get
        {
            return rotation * Vector3.right;
        }
        set
        {
            rotation = Quaternion.FromToRotation(Vector3.right, value);
        }
    }

    public Vector3 localRight
    {
        get
        {
            return localRotation * Vector3.right;
        }
        set
        {
            localRotation = Quaternion.FromToRotation(Vector3.right, value);
        }
    }

    public Vector3 up
    {
        get
        {
            return rotation * Vector3.up;
        }
        set
        {
            rotation = Quaternion.FromToRotation(Vector3.up, value);
        }
    }

    public Vector3 localUp
    {
        get
        {
            return localRotation * Vector3.up;
        }
        set
        {
            localRotation = Quaternion.FromToRotation(Vector3.up, value);
        }
    }


    public Vector3 forward
    {
        get
        {
            return rotation * Vector3.forward;
        }
        set
        {
            rotation = Quaternion.FromToRotation(Vector3.forward, value);
        }
    }

    public Vector3 localForward
    {
        get
        {
            return localRotation * Vector3.forward;
        }
        set
        {
            localRotation = Quaternion.FromToRotation(Vector3.forward, value);
        }
    }

    public void MoveBy(Vector3 amount)
    {
        internal_m2n_move_by_global(owner, amount);
    }

    public void MoveByLocal(Vector3 amount)
    {
        internal_m2n_move_by_local(owner, amount);
    }
    
    public void ScaleBy(Vector3 amount)
    {
        internal_m2n_scale_by_global(owner, amount);
    }

    public void ScaleByLocal(Vector3 amount)
    {
        internal_m2n_scale_by_local(owner, amount);
    }

    public void RotateBy(Quaternion amount)
    {
        internal_m2n_rotate_by_global(owner, amount);
    }

    public void RotateByLocal(Quaternion amount)
    {
        internal_m2n_rotate_by_local(owner, amount);
    }

    public void RotateByEuler(Vector3 amount)
    {
        internal_m2n_rotate_by_euler_global(owner, amount);
    }

    public void RotateByEulerLocal(Vector3 amount)
    {
        internal_m2n_rotate_by_euler_local(owner, amount);
    }

    public void RotateAxis(float degrees, Vector3 axis)
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
        LookAt(target.transform.position, up);
    }

    public void LookAt(TransformComponent target)
    {
        LookAt(target.position, Vector3.up);
    }

    public void LookAt(TransformComponent target, Vector3 up)
    {
        LookAt(target.position, up);
    }

    public void RotateAround(Vector3 point, Vector3 axis, float angle)
    {
        Vector3 vector = position;
        Quaternion quaternion = Quaternion.AngleAxis(angle, axis);
        Vector3 vector2 = vector - point;
        vector2 = quaternion * vector2;
        vector = point + vector2;
        position = vector;
        RotateAxis(angle, axis);
    }

    public void RotateAround(Entity target, Vector3 axis, float angle)
    {
        RotateAround(target.transform.position, axis, angle);
    }

    public void RotateAround(TransformComponent target, Vector3 axis, float angle)
    {
        RotateAround(target.position, axis, angle);
    }

    public void MoveTowards(Entity target, float maxDistanceDelta)
    {
        position = Vector3.MoveTowards(position, target.transform.position, maxDistanceDelta);
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


