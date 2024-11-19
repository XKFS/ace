using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public class PhysicsComponent : Component
{
    public void ApplyImpulse(Vector3 impulse)
    {
        internal_m2n_apply_impulse(owner, impulse);
    }

    public void ApplyTorqueImpulse(Vector3 impulse)
    {
        internal_m2n_apply_impulse(owner, impulse);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_apply_impulse(Entity eid, Vector3 impulse);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_apply_torque_impulse(Entity eid, Vector3 impulse);

    
}

}
}


