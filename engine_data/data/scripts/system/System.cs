using System;
using System.Runtime.CompilerServices;
using System.Reflection;
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

        // Get the assembly containing the internal calls
        Assembly assembly = Assembly.GetExecutingAssembly();

        // Iterate through all types in the assembly
        foreach (Type type in assembly.GetTypes())
        {
            // Iterate through all methods in the type
            foreach (MethodInfo method in type.GetMethods(BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static))
            {
                // Check if the method is internal
                if (method.IsAssembly)
                {
                    // Check if the method is bound
                    if (method.MethodImplementationFlags == MethodImplAttributes.InternalCall)
                    {
                        Console.WriteLine($"Internal call not bound: {method.Name} in {type.Name}");
                    }
                }
            }
        }
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


