using System;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Text;
using PInvoke;
using Serilog;
using SokkaServer.Exceptions;

namespace SokkaServer.Children
{
    internal class AirBenderDualShock3 : AirBenderChildDevice
    {
        private readonly byte[] _ledOffsets = { 0x02, 0x04, 0x08, 0x10 };

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

        public AirBenderDualShock3(AirBender host, PhysicalAddress client) : base(host, client)
        {
        }

        protected override void OnInputReport(long l)
        {
            int bytesReturned;
            var requestSize = Marshal.SizeOf<AirBender.AIRBENDER_GET_DS3_INPUT_REPORT>();
            var requestBuffer = Marshal.AllocHGlobal(requestSize);

            Marshal.StructureToPtr(
                new AirBender.AIRBENDER_GET_DS3_INPUT_REPORT
                {
                    ClientAddress = ClientAddress.ToNativeBdAddr()
                },
                requestBuffer, false);

            var ret = Kernel32.DeviceIoControl(
                HostDevice.DeviceHandle,
                unchecked((int)AirBender.IOCTL_AIRBENDER_GET_DS3_INPUT_REPORT),
                requestBuffer, requestSize, requestBuffer, requestSize,
                out bytesReturned, IntPtr.Zero);

            if (!ret && Marshal.GetLastWin32Error() == AirBender.ERROR_DEV_NOT_EXIST)
            {
                Marshal.FreeHGlobal(requestBuffer);

                throw new AirBenderDeviceNotFoundException();
            }

            if (ret)
            {
                var resp = Marshal.PtrToStructure<AirBender.AIRBENDER_GET_DS3_INPUT_REPORT>(requestBuffer);

                var sb = new StringBuilder();

                foreach (var b in resp.ReportBuffer)
                {
                    sb.Append($"{b:X2} ");
                }

                Log.Information(sb.ToString());
            }

            Marshal.FreeHGlobal(requestBuffer);
        }

        protected override void OnOutputReport(long l)
        {
            int bytesReturned;
            var requestSize = Marshal.SizeOf<AirBender.AIRBENDER_SET_DS3_OUTPUT_REPORT>();
            var requestBuffer = Marshal.AllocHGlobal(requestSize);

            Marshal.StructureToPtr(
                new AirBender.AIRBENDER_SET_DS3_OUTPUT_REPORT()
                {
                    ClientAddress = ClientAddress.ToNativeBdAddr(),
                    ReportBuffer = _hidOutputReport
                },
                requestBuffer, false);

            var ret = Kernel32.DeviceIoControl(
                HostDevice.DeviceHandle,
                unchecked((int)AirBender.IOCTL_AIRBENDER_SET_DS3_OUTPUT_REPORT),
                requestBuffer, requestSize, IntPtr.Zero, 0,
                out bytesReturned, IntPtr.Zero);

            if (!ret && Marshal.GetLastWin32Error() == AirBender.ERROR_DEV_NOT_EXIST)
            {
                Marshal.FreeHGlobal(requestBuffer);

                throw new AirBenderDeviceNotFoundException();
            }

            Marshal.FreeHGlobal(requestBuffer);
        }
    }
}
