using System;
using TinyIpc.Messaging;

namespace AirBender.Common.Shared.Core
{
    public class MessageHub
    {
        private static readonly Lazy<TinyMessageBus> DeviceArrivalBusLazy =
            new Lazy<TinyMessageBus>(() => new TinyMessageBus("AirBenderDeviceArrivalBus"));

        private static readonly Lazy<TinyMessageBus> DeviceRemovalBusLazy =
            new Lazy<TinyMessageBus>(() => new TinyMessageBus("AirBenderDeviceRemovalBus"));

        private static readonly Lazy<TinyMessageBus> InputReportBusLazy =
            new Lazy<TinyMessageBus>(() => new TinyMessageBus("AirBenderInputReportBus"));

        private static readonly Lazy<TinyMessageBus> OutputReportBusLazy =
            new Lazy<TinyMessageBus>(() => new TinyMessageBus("AirBenderOutputReportBus"));

        public static TinyMessageBus DeviceArrivalBus => DeviceArrivalBusLazy.Value;

        public static TinyMessageBus DeviceRemovalBus => DeviceRemovalBusLazy.Value;

        public static TinyMessageBus InputReportBus => InputReportBusLazy.Value;

        public static TinyMessageBus OutputReportBus => OutputReportBusLazy.Value;
    }
}