using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{


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


