using System;
using System.Runtime.InteropServices;

namespace SokkaServer
{
    partial class AirBender
    {
        private const uint IOCTL_AIRBENDER_GET_HOST_BD_ADDR = 0x80006004;
        private const uint IOCTL_AIRBENDER_HOST_RESET = 0x80002008;
        private const uint IOCTL_AIRBENDER_GET_CLIENT_COUNT = 0x8000600C;
        private const uint IOCTL_AIRBENDER_GET_CLIENT_STATE = 0x8000E010;
        internal const uint IOCTL_AIRBENDER_GET_DS3_INPUT_REPORT = 0x8000E014;
        internal const uint IOCTL_AIRBENDER_SET_DS3_OUTPUT_REPORT = 0x8000A018;
        internal const uint IOCTL_AIRBENDER_HOST_SHUTDOWN = 0x8000201C;

        internal const int ERROR_DEV_NOT_EXIST = 0x37;

        private const int DS3_HID_INPUT_REPORT_SIZE = 0x31;
        private const int DS3_HID_OUTPUT_REPORT_SIZE = 0x32;

        [StructLayout(LayoutKind.Sequential)]
        internal struct BD_ADDR
        {
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
            public byte[] Address;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct AIRBENDER_GET_HOST_BD_ADDR
        {
            public BD_ADDR Host;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct AIRBENDER_GET_CLIENT_COUNT
        {
            public UInt32 Count;
        }

        public enum BTH_DEVICE_TYPE : UInt32
        {
            DualShock3,
            DualShock4,
            Unknown
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct AIRBENDER_GET_CLIENT_DETAILS
        {
            public UInt32 ClientIndex;
            public BTH_DEVICE_TYPE DeviceType;
            public BD_ADDR ClientAddress;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        internal struct AIRBENDER_GET_DS3_INPUT_REPORT
        {
            public BD_ADDR ClientAddress;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = DS3_HID_INPUT_REPORT_SIZE)]
            public byte[] ReportBuffer;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        internal struct AIRBENDER_SET_DS3_OUTPUT_REPORT
        {
            public BD_ADDR ClientAddress;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = DS3_HID_OUTPUT_REPORT_SIZE)]
            public byte[] ReportBuffer;
        }
    }
}
