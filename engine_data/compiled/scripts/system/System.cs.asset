using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{
[StructLayout(LayoutKind.Sequential)]
public struct UpdateInfo
{
    public float deltaTime;
    public float timeScale;
}

public static class Time
{
    public static float deltaTime;
    public static float timeScale;
}

public static class SystemManager
{
    public static event Action OnUpdate;

    public static void internal_n2m_update(UpdateInfo info)
    {
        Time.deltaTime = info.deltaTime;
        Time.timeScale = info.timeScale;

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


