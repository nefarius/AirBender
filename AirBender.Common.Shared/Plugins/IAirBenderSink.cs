using AirBender.Common.Shared.Core;

namespace AirBender.Common.Shared.Plugins
{
    public interface IAirBenderSink
    {
        void DeviceArrived(ChildDeviceState device);

        void DeviceRemoved(ChildDeviceState device);

        void InputReportReceived(ChildDeviceState device);
    }
}