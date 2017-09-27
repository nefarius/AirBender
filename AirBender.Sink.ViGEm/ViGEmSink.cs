using System.Collections.Generic;
using AirBender.Common.Shared.Messages;
using AirBender.Common.Shared.Serialization;
using TinyIpc.Messaging;

namespace AirBender.Sink.ViGEm
{
    internal class ViGEmSink
    {
        private readonly TinyMessageBus _deviceArrivalBus = new TinyMessageBus("AirBenderDeviceArrivalBus");
        private readonly TinyMessageBus _deviceRemovalBus = new TinyMessageBus("AirBenderDeviceRemovalBus");
        private readonly List<DeviceMetaMessage> _devices = new List<DeviceMetaMessage>();
        private readonly TinyMessageBus _inputReportBus = new TinyMessageBus("AirBenderInputReportBus");

        public void Start()
        {
            _deviceArrivalBus.MessageReceived +=
                (sender, args) => _devices.Add(MessageExtensions.ToObject<DeviceMetaMessage>(args.Message));

            _deviceRemovalBus.MessageReceived += 
                (sender, args) => _devices.Remove(MessageExtensions.ToObject<DeviceMetaMessage>(args.Message));

            _inputReportBus.MessageReceived += InputReportBusOnInputReportReceived;
        }

        private void InputReportBusOnInputReportReceived(object sender,
            TinyMessageReceivedEventArgs tinyMessageReceivedEventArgs)
        {
            var t = 0;
        }

        public void Stop()
        {
        }
    }
}