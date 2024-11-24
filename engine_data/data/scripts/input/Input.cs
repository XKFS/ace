using System;
using System.Runtime.CompilerServices;
namespace Ace
{
namespace Core
{
public static class Input
{
    public static float GetAxis(string action)
    {
        return GetAnalogValue(action);
    }

    public static bool GetButtonDown(string action)
    {
        return IsPressed(action);
    }
    
    public static bool GetButtonUp(string action)
    {
        return IsReleased(action);
    }

    public static bool GetButton(string action)
    {
        return IsDown(action);
    }

    public static float GetAnalogValue(string action)
    {
        return internal_m2n_input_get_analog_value(action);
    }

    public static bool GetDigitalValue(string action)
    {
        return internal_m2n_input_get_digital_value(action);
    }

    public static bool IsPressed(string action)
    {
        return internal_m2n_input_is_pressed(action);
    }

    public static bool IsReleased(string action)
    {
        return internal_m2n_input_is_released(action);
    }

    public static bool IsDown(string action)
    {
        return internal_m2n_input_is_down(action);
    }

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern float internal_m2n_input_get_analog_value(string action);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool internal_m2n_input_get_digital_value(string action);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool internal_m2n_input_is_pressed(string action);

    
    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool internal_m2n_input_is_released(string action);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern bool internal_m2n_input_is_down(string action);
}

}

}