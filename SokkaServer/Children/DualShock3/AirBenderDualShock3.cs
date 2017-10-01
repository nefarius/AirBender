using System;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Threading;
using AirBender.Common.Shared.Core;
using AirBender.Common.Shared.Reports.DualShock3;
using SokkaServer.Exceptions;
using SokkaServer.Host;
using SokkaServer.Util;

namespace SokkaServer.Children.DualShock3
{
    internal class AirBenderDualShock3 : AirBenderChildDevice
    {
        private readonly byte[] _hidOutputReport =
        {
            0x52, 0x01, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xFF, 0x27, 0x10, 0x00,
            0x32, 0xFF, 0x27, 0x10, 0x00, 0x32, 0xFF, 0x27,
            0x10, 0x00, 0x32, 0xFF, 0x27, 0x10, 0x00, 0x32,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00
        };

        private readonly byte[] _ledOffsets = { 0x02, 0x04, 0x08, 0x10 };

        public AirBenderDualShock3(AirBenderHost host, PhysicalAddress client, int index) : base(host, client, index)
        {
            DeviceType = BthDeviceType.DualShock3;

            if (index >= 0 && index < 4)
                _hidOutputReport[11] = _ledOffsets[index];
        }

        protected override void RequestInputReportWorker(object cancellationToken)
        {
            var token = (CancellationToken)cancellationToken;
            var requestSize = Marshal.SizeOf<AirBenderHost.AirbenderGetDs3InputReport>();
            var requestBuffer = Marshal.AllocHGlobal(requestSize);

            Marshal.StructureToPtr(
                new AirBenderHost.AirbenderGetDs3InputReport
                {
                    ClientAddress = ClientAddress.ToNativeBdAddr()
                },
                requestBuffer, false);

            try
            {
                while (!token.IsCancellationRequested)
                {
                    int bytesReturned;

                    //
                    // This call blocks until the driver supplies new data.
                    //  
                    var ret = HostDevice.DeviceHandle.OverlappedDeviceIoControl(
                        AirBenderHost.IoctlAirbenderGetDs3InputReport,
                        requestBuffer, requestSize, requestBuffer, requestSize,
                        out bytesReturned);

                    if (!ret && Marshal.GetLastWin32Error() == AirBenderHost.ErrorDevNotExist)
                        OnChildDeviceDisconnected(EventArgs.Empty);

                    if (!ret) continue;

                    var resp = Marshal.PtrToStructure<AirBenderHost.AirbenderGetDs3InputReport>(requestBuffer);

                    OnInputReport(new DualShock3InputReport(resp.ReportBuffer));
                }
            }
            finally
            {
                Marshal.FreeHGlobal(requestBuffer);
            }
        }

        protected override void OnOutputReport(long l)
        {
            int bytesReturned;
            var requestSize = Marshal.SizeOf<AirBenderHost.AirbenderSetDs3OutputReport>();
            var requestBuffer = Marshal.AllocHGlobal(requestSize);

            Marshal.StructureToPtr(
                new AirBenderHost.AirbenderSetDs3OutputReport
                {
                    ClientAddress = ClientAddress.ToNativeBdAddr(),
                    ReportBuffer = _hidOutputReport
                },
                requestBuffer, false);

            var ret = HostDevice.DeviceHandle.OverlappedDeviceIoControl(
                AirBenderHost.IoctlAirbenderSetDs3OutputReport,
                requestBuffer, requestSize, IntPtr.Zero, 0,
                out bytesReturned);

            if (!ret && Marshal.GetLastWin32Error() == AirBenderHost.ErrorDevNotExist)
            {
                Marshal.FreeHGlobal(requestBuffer);
                throw new AirBenderDeviceNotFoundException();
            }

            Marshal.FreeHGlobal(requestBuffer);
        }
    }
}