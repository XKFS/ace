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
    public struct Ray : IFormattable
    {
        public Vector3 origin;
        public Vector3 direction;

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

            return string.Format(CultureInfo.InvariantCulture.NumberFormat, "(origin={0}, direction={1})",
                origin.ToString(format, formatProvider),
                direction.ToString(format, formatProvider));
        }
    };
    
}
}
