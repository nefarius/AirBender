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

        [StructLayout(LayoutKind.Sequential)]
        private struct BD_ADDR
        {
			[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
            public byte[] Address;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct AIRBENDER_GET_HOST_BD_ADDR
        {
            public BD_ADDR Host;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct AIRBENDER_GET_CLIENT_COUNT
        {
            public UInt32 Count;
        }

        public enum BTH_DEVICE_TYPE : uint
        {
            DualShock3,
            DualShock4
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct AIRBENDER_GET_CLIENT_STATE_REQUEST
        {
            public UInt32 ClientIndex;

            public UInt32 ResponseBufferSize;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct AIRBENDER_GET_CLIENT_STATE_RESPONSE
        {
            UInt32 ClientIndex;

            BTH_DEVICE_TYPE DeviceType;

            UInt32 ResponseBufferSize;

            UInt32 ResponseBuffer;
        }
    }
}
