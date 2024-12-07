using System;
using System.Globalization;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Ace
{
namespace Core
{

    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastHit : IFormattable
    {
        public Entity entity;
        public Vector3 point;
        public Vector3 normal;
        public float distance;

        public override string ToString()
        {
            return ToString(null, null);
        }

        public string ToString(string format)
        {
            return ToString(format, null);
        }


        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (string.IsNullOrEmpty(format))
            {
                format = "F2";
            }

            if (formatProvider == null)
            {
                formatProvider = CultureInfo.InvariantCulture.NumberFormat;
            }

            return string.Format(CultureInfo.InvariantCulture.NumberFormat, "(entity={0}, point={1}, normal={2}, distance={3})",
                entity.tag,
                point.ToString(format, formatProvider),
                normal.ToString(format, formatProvider),
                distance.ToString(format, formatProvider));
        }
    };
    
}
}
