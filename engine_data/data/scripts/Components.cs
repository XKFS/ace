using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public abstract class ScriptComponent
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


