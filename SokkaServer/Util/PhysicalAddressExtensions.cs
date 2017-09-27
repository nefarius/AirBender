using System.Linq;
using System.Net.NetworkInformation;
using SokkaServer.Host;

namespace SokkaServer.Util
{
    public static class PhysicalAddressExtensions
    {
        public static string AsFriendlyName(this PhysicalAddress address)
        {
            if (address == null)
                return string.Empty;

            if (address.Equals(PhysicalAddress.None))
                return "00:00:00:00:00:00";

            var bytes = address.GetAddressBytes();

            return $"{bytes[0]:X2}:{bytes[1]:X2}:{bytes[2]:X2}:{bytes[3]:X2}:{bytes[4]:X2}:{bytes[5]:X2}";
        }

        internal static AirBender.BD_ADDR ToNativeBdAddr(this PhysicalAddress address)
        {
            return new AirBender.BD_ADDR()
            {
                Address = address.GetAddressBytes().Reverse().ToArray()
            };
        }
    }
}
