using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public abstract class Component : NativeObject
{
    public Entity Owner { get; internal set; }
    private void Internal_SetEntity(uint id, bool valid)
    {
        Owner = new Entity(id);
        m_alive = valid;
    }
}

public class TransformComponent : Component
{

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

    public abstract void OnCreate();
    public abstract void OnStart();
    public abstract void OnUpdate();


}

}
}


