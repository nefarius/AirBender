using AirBender.Common.Shared.Core;
using AirBender.Common.Shared.Reports;

namespace AirBender.Common.Shared.Plugins
{
    public interface IAirBenderSink
    {
        void DeviceArrived(IAirBenderChildDevice device);

        void DeviceRemoved(IAirBenderChildDevice device);

        void InputReportReceived(IAirBenderChildDevice device, IInputReport report);
    }
}