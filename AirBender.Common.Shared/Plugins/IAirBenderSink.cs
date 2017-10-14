using AirBender.Common.Shared.Core;
using Nefarius.Sub.Kinbaku.Core.Reports.Common;

namespace AirBender.Common.Shared.Plugins
{
    public interface IAirBenderSink
    {
        void DeviceArrived(IAirBenderChildDevice device);

        void DeviceRemoved(IAirBenderChildDevice device);

        void InputReportReceived(IAirBenderChildDevice device, IInputReport report);
    }
}