using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{

public static class SystemManager
{
    public static event Action OnUpdate;

    public static void internal_n2m_update()
    {
        OnUpdate?.Invoke();
    }
}

public abstract class ScriptSystem
{
    public ScriptSystem()
    {
        SystemManager.OnUpdate += OnUpdate;
    }

    ~ScriptSystem()
    {
        SystemManager.OnUpdate -= OnUpdate;
    }

    public abstract void OnCreate();
    public abstract void OnStart();
    public abstract void OnUpdate();


    private void internal_n2m_on_create()
    {
        OnCreate();
    }
    private void internal_n2m_on_start()
    {
        OnStart();
    }
}


}

}


