using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public class AnimationComponent : Component
{

    public void Blend(AnimationClip clip, float seconds)
    {
        internal_m2n_animation_blend(owner, clip.uid, seconds);
    }

    public void Play()
    {
        internal_m2n_animation_play(owner);
    }

    public void Pause()
    {
        internal_m2n_animation_pause(owner);
    }

    public void Resume()
    {
        internal_m2n_animation_resume(owner);
    }

    public void Stop()
    {
        internal_m2n_animation_stop(owner);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_animation_blend(Entity eid, Guid guid, float seconds);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_animation_play(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_animation_pause(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_animation_resume(Entity eid);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void internal_m2n_animation_stop(Entity eid);
}

}
}


