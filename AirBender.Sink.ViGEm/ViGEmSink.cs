using System.Collections.Generic;
using System.Linq;
using AirBender.Common.Shared.Core;
using AirBender.Common.Shared.Messages;
using AirBender.Common.Shared.Reports.DualShock3;
using AirBender.Common.Shared.Serialization;
using Nefarius.ViGEm.Client;
using Nefarius.ViGEm.Client.Targets;
using Nefarius.ViGEm.Client.Targets.DualShock4;
using Serilog;
using TinyIpc.Messaging;

namespace AirBender.Sink.ViGEm
{
    internal class ViGEmSink
    {
        private readonly Dictionary<DeviceMetaMessage, DualShock4Controller> _deviceMap =
            new Dictionary<DeviceMetaMessage, DualShock4Controller>();

        private Dictionary<DualShock3Buttons, DualShock4Buttons> _btnMap;
        private ViGEmClient _client;

        public void Start()
        {
            _btnMap = new Dictionary<DualShock3Buttons, DualShock4Buttons>
            {
                {DualShock3Buttons.Select, DualShock4Buttons.Share},
                {DualShock3Buttons.LeftThumb, DualShock4Buttons.ThumbLeft},
                {DualShock3Buttons.RightThumb, DualShock4Buttons.ThumbRight},
                {DualShock3Buttons.Start, DualShock4Buttons.Options},
                {DualShock3Buttons.LeftTrigger, DualShock4Buttons.TriggerLeft},
                {DualShock3Buttons.RightTrigger, DualShock4Buttons.TriggerRight},
                {DualShock3Buttons.LeftShoulder, DualShock4Buttons.ShoulderLeft},
                {DualShock3Buttons.RightShoulder, DualShock4Buttons.ShoulderRight},
                {DualShock3Buttons.Triangle, DualShock4Buttons.Triangle},
                {DualShock3Buttons.Circle, DualShock4Buttons.Circle},
                {DualShock3Buttons.Cross, DualShock4Buttons.Cross},
                {DualShock3Buttons.Square, DualShock4Buttons.Square}
            };

            _client = new ViGEmClient();

            MessageHub.DeviceArrivalBus.MessageReceived += (sender, args) =>
            {
                var source = MessageExtensions.ToObject<DeviceMetaMessage>(args.Message);
                var target = new DualShock4Controller(_client);

                Log.Information($"New {source.DeviceType} device arrived (#{source.Index})");

                _deviceMap.Add(source, target);

                target.Connect();
            };

            MessageHub.DeviceRemovalBus.MessageReceived += (sender, args) =>
            {
                var source = MessageExtensions.ToObject<DeviceMetaMessage>(args.Message);

                Log.Information($"{source.DeviceType} device removed (#{source.Index})");

                _deviceMap[source].Dispose();
                _deviceMap.Remove(source);
            };

            MessageHub.InputReportBus.MessageReceived += InputReportBusOnInputReportReceived;
        }

        private void InputReportBusOnInputReportReceived(object sender,
            TinyMessageReceivedEventArgs tinyMessageReceivedEventArgs)
        {
            var source = MessageExtensions.ToObject<DeviceMetaMessage>(tinyMessageReceivedEventArgs.Message);

            switch (source.DeviceType)
            {
                case BthDeviceType.DualShock3:

                    var target = _deviceMap[source];
                    var report = new DualShock3InputReport(source.InputReport);

                    var ds4Report = new DualShock4Report();

                    ds4Report.SetAxis(DualShock4Axes.LeftThumbX, report.LeftThumbX);
                    ds4Report.SetAxis(DualShock4Axes.LeftThumbY, report.LeftThumbY);
                    ds4Report.SetAxis(DualShock4Axes.RightThumbX, report.RightThumbX);
                    ds4Report.SetAxis(DualShock4Axes.RightThumbY, report.RightThumbY);
                    ds4Report.SetAxis(DualShock4Axes.LeftTrigger, report.LeftTrigger);
                    ds4Report.SetAxis(DualShock4Axes.RightTrigger, report.RightTrigger);

                    foreach (var engagedButton in report.EngagedButtons)
                        ds4Report.SetButtons(_btnMap.Where(m => m.Key == engagedButton).Select(m => m.Value)
                            .ToArray());

                    if (report.EngagedButtons.Contains(DualShock3Buttons.DPadUp))
                        ds4Report.SetDPad(DualShock4DPadValues.North);
                    if (report.EngagedButtons.Contains(DualShock3Buttons.DPadRight))
                        ds4Report.SetDPad(DualShock4DPadValues.East);
                    if (report.EngagedButtons.Contains(DualShock3Buttons.DPadDown))
                        ds4Report.SetDPad(DualShock4DPadValues.South);
                    if (report.EngagedButtons.Contains(DualShock3Buttons.DPadLeft))
                        ds4Report.SetDPad(DualShock4DPadValues.West);

                    if (report.EngagedButtons.Contains(DualShock3Buttons.DPadUp)
                        && report.EngagedButtons.Contains(DualShock3Buttons.DPadRight))
                        ds4Report.SetDPad(DualShock4DPadValues.Northeast);
                    if (report.EngagedButtons.Contains(DualShock3Buttons.DPadRight)
                        && report.EngagedButtons.Contains(DualShock3Buttons.DPadDown))
                        ds4Report.SetDPad(DualShock4DPadValues.Southeast);
                    if (report.EngagedButtons.Contains(DualShock3Buttons.DPadDown)
                        && report.EngagedButtons.Contains(DualShock3Buttons.DPadLeft))
                        ds4Report.SetDPad(DualShock4DPadValues.Southwest);
                    if (report.EngagedButtons.Contains(DualShock3Buttons.DPadLeft)
                        && report.EngagedButtons.Contains(DualShock3Buttons.DPadUp))
                        ds4Report.SetDPad(DualShock4DPadValues.Northwest);

                    if (report.EngagedButtons.Contains(DualShock3Buttons.Ps))
                        ds4Report.SetSpecialButtons(DualShock4SpecialButtons.Ps);

                    target.SendReport(ds4Report);

                    break;
            }
        }

        public void Stop()
        {
        }
    }
}