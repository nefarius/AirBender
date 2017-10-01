using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using AirBender.Common.Shared.Core;
using AirBender.Common.Shared.Plugins;
using AirBender.Common.Shared.Reports;
using AirBender.Common.Shared.Reports.DualShock3;
using Nefarius.ViGEm.Client;
using Nefarius.ViGEm.Client.Targets;
using Nefarius.ViGEm.Client.Targets.DualShock4;

namespace AirBender.Plugin.Sink.ViGEm
{
    [Export(typeof(IAirBenderSink))]
    public class ViGEmSink : IAirBenderSink
    {
        private readonly Dictionary<DualShock3Buttons, DualShock4Buttons> _btnMap;
        private readonly ViGEmClient _client;

        private readonly Dictionary<IAirBenderChildDevice, DualShock4Controller> _deviceMap =
            new Dictionary<IAirBenderChildDevice, DualShock4Controller>();

        public ViGEmSink()
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
        }

        public void DeviceArrived(IAirBenderChildDevice device)
        {
            var target = new DualShock4Controller(_client);

            _deviceMap.Add(device, target);

            target.Connect();
        }

        public void DeviceRemoved(IAirBenderChildDevice device)
        {
            _deviceMap[device].Dispose();
            _deviceMap.Remove(device);
        }

        public void InputReportReceived(IAirBenderChildDevice device, IInputReport report)
        {
            switch (device.DeviceType)
            {
                case BthDeviceType.DualShock3:

                    var target = _deviceMap[device];

                    var ds3Report = (DualShock3InputReport) report;
                    var ds4Report = new DualShock4Report();

                    ds4Report.SetAxis(DualShock4Axes.LeftThumbX, ds3Report.LeftThumbX);
                    ds4Report.SetAxis(DualShock4Axes.LeftThumbY, ds3Report.LeftThumbY);
                    ds4Report.SetAxis(DualShock4Axes.RightThumbX, ds3Report.RightThumbX);
                    ds4Report.SetAxis(DualShock4Axes.RightThumbY, ds3Report.RightThumbY);
                    ds4Report.SetAxis(DualShock4Axes.LeftTrigger, ds3Report.LeftTrigger);
                    ds4Report.SetAxis(DualShock4Axes.RightTrigger, ds3Report.RightTrigger);

                    ds4Report.SetButtons(_btnMap.Where(m => ds3Report.EngagedButtons.Contains(m.Key))
                        .Select(m => m.Value).ToArray());

                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadUp))
                        ds4Report.SetDPad(DualShock4DPadValues.North);
                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadRight))
                        ds4Report.SetDPad(DualShock4DPadValues.East);
                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadDown))
                        ds4Report.SetDPad(DualShock4DPadValues.South);
                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadLeft))
                        ds4Report.SetDPad(DualShock4DPadValues.West);

                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadUp)
                        && ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadRight))
                        ds4Report.SetDPad(DualShock4DPadValues.Northeast);
                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadRight)
                        && ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadDown))
                        ds4Report.SetDPad(DualShock4DPadValues.Southeast);
                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadDown)
                        && ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadLeft))
                        ds4Report.SetDPad(DualShock4DPadValues.Southwest);
                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadLeft)
                        && ds3Report.EngagedButtons.Contains(DualShock3Buttons.DPadUp))
                        ds4Report.SetDPad(DualShock4DPadValues.Northwest);

                    if (ds3Report.EngagedButtons.Contains(DualShock3Buttons.Ps))
                        ds4Report.SetSpecialButtons(DualShock4SpecialButtons.Ps);

                    target.SendReport(ds4Report);

                    break;
            }
        }
    }
}