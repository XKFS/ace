using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{
    /// <summary>
    /// Represents a component that provides model rendering capabilities for an entity.
    /// </summary>
    public class ModelComponent : Component
    {
        /// <summary>
        /// Gets or sets a value indicating whether the model is enabled.
        /// </summary>
        /// <value>
        /// <c>true</c> if the model is enabled; otherwise, <c>false</c>.
        /// </value>
        public bool enabled
        {
            get
            {
                return internal_m2n_model_get_enabled(owner);
            }
            set
            {
                internal_m2n_model_set_enabled(owner, value);
            }
        }

        /// <summary>
        /// Retrieves the material assigned to a specific index in the model.
        /// </summary>
        /// <param name="index">The index of the material to retrieve.</param>
        /// <returns>
        /// The <see cref="Material"/> assigned to the specified index, or <c>null</c> if no material is assigned.
        /// </returns>
        public Material GetMaterial(uint index)
        {
            var uid = internal_m2n_model_get_material(owner, index);

            if (uid == Guid.Empty)
            {
                return null;
            }

            return new Material { uid = uid };
        }

        /// <summary>
        /// Sets the material at the specified index for the model.
        /// </summary>
        /// <param name="material">The <see cref="Material"/> to assign, or <c>null</c> to remove the material.</param>
        /// <param name="index">The index of the material to set.</param>
        public void SetMaterial(Material material, uint index)
        {
            internal_m2n_model_set_material(owner, material?.uid ?? Guid.Empty, index);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool internal_m2n_model_get_enabled(Entity eid);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void internal_m2n_model_set_enabled(Entity eid, bool enabled);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Guid internal_m2n_model_get_material(Entity eid, uint index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void internal_m2n_model_set_material(Entity eid, Guid guid, uint index);
    }
}
}
