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
}


}

}


