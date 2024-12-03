using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{

public class Assets
{
   
    public static T GetAsset<T>(string key) where T : Asset<T>, new()
    {
        var asset_uid = internal_m2n_get_asset_by_key(key, typeof(T));
        T asset = new T();
        asset.uid = asset_uid;
        return asset;
    }

    public static T GetAsset<T>(Guid uid) where T : Asset<T>, new()
    {
        var asset_uid = internal_m2n_get_asset_by_uuid(uid, typeof(T));
        T asset = new T();
        asset.uid = asset_uid;
        return asset;
    }

    
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Guid internal_m2n_get_asset_by_uuid(Guid uid, Type obj);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern Guid internal_m2n_get_asset_by_key(string key, Type obj);

}

public class Texture : Asset<Texture>
{
}

public class Material : Asset<Material>
{
}

public class Mesh : Asset<Mesh>
{
}

public class AnimationClip : Asset<AnimationClip>
{
}


public class PhysicsMaterial : Asset<PhysicsMaterial>
{
}

public class AudioClip : Asset<AudioClip>
{
    //
    // Summary:
    //     The length of the audio clip in seconds. (Read Only)
    public float length
    {
        get
        {
            return internal_m2n_audio_clip_get_length(uid);
        }
    }

    //
    // Summary:
    //     The length of the audio clip in samples. (Read Only)
    // public extern int samples
    // {
    //     [MethodImpl(MethodImplOptions.InternalCall)]
    //     get;
    // }

    // //
    // // Summary:
    // //     The number of channels in the audio clip. (Read Only)
    // public extern int channels
    // {
    //     [MethodImpl(MethodImplOptions.InternalCall)]
    //     get;
    // }

    // //
    // // Summary:
    // //     The sample frequency of the clip in Hertz. (Read Only)
    // public extern int frequency
    // {
    //     [MethodImpl(MethodImplOptions.InternalCall)]
    //     get;
    // }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float internal_m2n_audio_clip_get_length(Guid uid);
}

}

}


