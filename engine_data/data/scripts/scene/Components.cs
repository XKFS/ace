using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public abstract class Component : NativeObject
{
    public Entity owner { get; internal set; }

    public TransformComponent transform
    {
        get
        {
            return owner.GetComponent<TransformComponent>();
        }
    }
    public override bool IsValid()
    {
        if(!owner.IsValid())
        {
            return false;
        }
        return owner.HasComponent(GetType());
    }
    private void internal_n2m_set_entity(uint id)
    {
        owner = new Entity(id);
    }
}

}
}


