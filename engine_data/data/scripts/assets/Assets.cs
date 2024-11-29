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
}

}

}


