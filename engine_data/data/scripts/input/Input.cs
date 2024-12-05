using System;
using System.Runtime.CompilerServices;

namespace Ace.Core
{
    /// <summary>
    /// Provides static methods to handle user input actions such as button presses and axis values.
    /// </summary>
    public static class Input
    {
        /// <summary>
        /// Gets the analog value of an input axis.
        /// </summary>
        /// <param name="action">The name of the action or axis to query.</param>
        /// <returns>The analog value of the specified axis.</returns>
        public static float GetAxis(string action)
        {
            return GetAnalogValue(action);
        }

        /// <summary>
        /// Checks if the specified action's button was pressed during the current frame.
        /// </summary>
        /// <param name="action">The name of the action or button to query.</param>
        /// <returns><c>true</c> if the button was pressed during this frame; otherwise, <c>false</c>.</returns>
        public static bool GetButtonDown(string action)
        {
            return IsPressed(action);
        }

        /// <summary>
        /// Checks if the specified action's button was released during the current frame.
        /// </summary>
        /// <param name="action">The name of the action or button to query.</param>
        /// <returns><c>true</c> if the button was released during this frame; otherwise, <c>false</c>.</returns>
        public static bool GetButtonUp(string action)
        {
            return IsReleased(action);
        }

        /// <summary>
        /// Checks if the specified action's button is currently being held down.
        /// </summary>
        /// <param name="action">The name of the action or button to query.</param>
        /// <returns><c>true</c> if the button is currently being held down; otherwise, <c>false</c>.</returns>
        public static bool GetButton(string action)
        {
            return IsDown(action);
        }

        /// <summary>
        /// Gets the analog value associated with the specified input action.
        /// </summary>
        /// <param name="action">The name of the action to query.</param>
        /// <returns>The analog value associated with the action.</returns>
        public static float GetAnalogValue(string action)
        {
            return internal_m2n_input_get_analog_value(action);
        }

        /// <summary>
        /// Gets the digital (boolean) value associated with the specified input action.
        /// </summary>
        /// <param name="action">The name of the action to query.</param>
        /// <returns><c>true</c> if the action is active; otherwise, <c>false</c>.</returns>
        public static bool GetDigitalValue(string action)
        {
            return internal_m2n_input_get_digital_value(action);
        }

        /// <summary>
        /// Checks if the specified action's button was pressed during the current frame.
        /// </summary>
        /// <param name="action">The name of the action to query.</param>
        /// <returns><c>true</c> if the button was pressed during this frame; otherwise, <c>false</c>.</returns>
        public static bool IsPressed(string action)
        {
            return internal_m2n_input_is_pressed(action);
        }

        /// <summary>
        /// Checks if the specified action's button was released during the current frame.
        /// </summary>
        /// <param name="action">The name of the action to query.</param>
        /// <returns><c>true</c> if the button was released during this frame; otherwise, <c>false</c>.</returns>
        public static bool IsReleased(string action)
        {
            return internal_m2n_input_is_released(action);
        }

        /// <summary>
        /// Checks if the specified action's button is currently being held down.
        /// </summary>
        /// <param name="action">The name of the action to query.</param>
        /// <returns><c>true</c> if the button is currently being held down; otherwise, <c>false</c>.</returns>
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
