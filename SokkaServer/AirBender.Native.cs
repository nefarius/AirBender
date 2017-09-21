using System;
using System.Runtime.InteropServices;

namespace SokkaServer
{
    partial class AirBender
    {
        private const uint IOCTL_AIRBENDER_GET_HOST_BD_ADDR = 0xFFDC6000;

        [StructLayout(LayoutKind.Sequential)]
        private struct BD_ADDR
        {
			[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
            public byte[] Address;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct AIRBENDER_GET_HOST_BD_ADDR
        {
            public UInt32 Size;

            public BD_ADDR Host;
        }
    }
}
