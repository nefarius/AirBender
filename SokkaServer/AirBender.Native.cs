using System;
using System.Runtime.InteropServices;

namespace SokkaServer
{
    partial class AirBender
    {
        private const uint IOCTL_AIRBENDER_GET_HOST_BD_ADDR = 0x80006004;
        private const uint IOCTL_AIRBENDER_HOST_RESET = 0x80002008;
        private const uint IOCTL_AIRBENDER_GET_CLIENT_COUNT = 0x8000600C;

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
    }
}
