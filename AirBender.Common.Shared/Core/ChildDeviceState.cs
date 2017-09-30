using System.Net.NetworkInformation;
using AirBender.Common.Shared.Reports;

namespace AirBender.Common.Shared.Core
{
    public class ChildDeviceState
    {
        public int Index { get; set; }

        public PhysicalAddress Address { get; set; }

        public BthDeviceType DeviceType { get; set; }

        public IInputReport InputReport { get; set; }

        public override bool Equals(object obj)
        {
            return Equals(obj as ChildDeviceState);
        }

        protected bool Equals(ChildDeviceState other)
        {
            return Index == other.Index && DeviceType == other.DeviceType;
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = Index;
                hashCode = (hashCode * 397) ^ (int) DeviceType;
                return hashCode;
            }
        }

        public static bool operator ==(ChildDeviceState left, ChildDeviceState right)
        {
            return Equals(left, right);
        }

        public static bool operator !=(ChildDeviceState left, ChildDeviceState right)
        {
            return !Equals(left, right);
        }
    }
}