using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
public abstract class Component : NativeObject
{
    public override bool IsValid()
    {
        if(!owner.IsValid())
        {
            return false;
        }
        return owner.HasComponent(GetType());
    }
    public Entity owner { get; internal set; }
    private void internal_n2m_set_entity(uint id)
    {
        owner = new Entity(id);
    }
}

}
}


