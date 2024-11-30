using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{


public abstract class ScriptComponent : Component
{
    public virtual void OnCreate()
    {

    }
    public virtual void OnStart()
    {

    }

    public virtual void OnDestroy()
    {

    }

    public virtual void OnSensorEnter(Entity e)
    {

    }
    public virtual void OnSensorExit(Entity e)
    {
        
    }

    public virtual void OnCollisionEnter(Collision collision)
    {

    }
    public virtual void OnCollisionExit(Collision collision)
    {
        
    }

    public virtual void OnUpdate()
    {

    }


    private void internal_n2m_on_create()
    {
        OnCreate();
        SystemManager.OnUpdate += OnUpdate;
    }
    private void internal_n2m_on_start()
    {
        OnStart();

    }

    private void internal_n2m_on_destroy()
    {
        SystemManager.OnUpdate -= OnUpdate;
        OnDestroy();
    }

    private void internal_n2m_on_sensor_enter(Entity entity)
    {
        OnSensorEnter(entity);
    }

    private void internal_n2m_on_sensor_exit(Entity entity)
    {
        OnSensorExit(entity);
    }

    private void internal_n2m_on_collision_enter(Entity entity, byte[] contactData)
    // private void internal_n2m_on_collision_enter(Entity entity)
    {
        Collision collision = new Collision();
        collision.entity = entity;
        collision.contacts = contactData.ToStructArray<ContactPoint>();

        OnCollisionEnter(collision);
    }

    private void internal_n2m_on_collision_exit(Entity entity, byte[] contactData)
    // private void internal_n2m_on_collision_exit(Entity entity)
    {
        Collision collision = new Collision();
        collision.entity = entity;
        collision.contacts = contactData.ToStructArray<ContactPoint>();

        OnCollisionExit(collision);
    }
}

}
}


