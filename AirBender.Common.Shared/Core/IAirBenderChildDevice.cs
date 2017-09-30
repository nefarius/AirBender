using System.Net.NetworkInformation;

namespace AirBender.Common.Shared.Core
{
    public interface IAirBenderChildDevice
    {
        int DeviceIndex { get; }

        PhysicalAddress ClientAddress { get; }

        BthDeviceType DeviceType { get; }
    }
}