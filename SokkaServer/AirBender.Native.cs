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

        private const int ERROR_DEV_NOT_EXIST = 0x37;

        [StructLayout(LayoutKind.Sequential)]
        private struct BD_ADDR
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
            DualShock4
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct AIRBENDER_GET_CLIENT_DETAILS
        {
            public UInt32 ClientIndex;
            public BTH_DEVICE_TYPE DeviceType;
            public BD_ADDR ClientAddress;
        }
    }
}
