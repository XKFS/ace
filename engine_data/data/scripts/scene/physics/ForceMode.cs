using System;
using System.Globalization;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{
    /// <summary>
    /// Specifies how forces are applied to physics components.
    /// </summary>
    public enum ForceMode : byte
    {
        /// <summary>
        /// Interprets the input as torque (measured in Newton-metres), and changes the angular velocity by the value of 
        /// torque * deltaTime / mass. The effect depends on the simulation step length and the mass of the body.
        /// </summary>
        Force,

        /// <summary>
        /// Interprets the parameter as angular acceleration (measured in degrees per second squared), and changes the
        /// angular velocity by the value of torque * deltaTime. The effect depends on the simulation step length but
        /// does not depend on the mass of the body.
        /// </summary>
        Acceleration,

        /// <summary>
        /// Interprets the parameter as angular momentum (measured in kilogram-meters-squared per second), and changes the
        /// angular velocity by the value of torque / mass. The effect depends on the mass of the body but doesn't depend 
        /// on the simulation step length.
        /// </summary>
        Impulse,

        /// <summary>
        /// Interprets the parameter as a direct angular velocity change (measured in degrees per second), and changes the
        /// angular velocity by the value of torque. The effect doesn't depend on the mass of the body or the simulation 
        /// step length.
        /// </summary>
        VelocityChange
    }

}
}
