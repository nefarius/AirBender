using System;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using PInvoke;

namespace SokkaServer
{
    partial class AirBender
    {
        public static Guid ClassGuid => Guid.Parse("a775e97e-a41b-4bfc-868e-25be84643b62");

        public AirBender(string devicePath)
        {
            DevicePath = devicePath;

            DeviceHandle = Kernel32.CreateFile(DevicePath,
                Kernel32.ACCESS_MASK.GenericRight.GENERIC_READ | Kernel32.ACCESS_MASK.GenericRight.GENERIC_WRITE,
                Kernel32.FileShare.FILE_SHARE_READ | Kernel32.FileShare.FILE_SHARE_WRITE,
                IntPtr.Zero, Kernel32.CreationDisposition.OPEN_EXISTING,
                Kernel32.CreateFileFlags.FILE_ATTRIBUTE_NORMAL /*| Kernel32.CreateFileFlags.FILE_FLAG_OVERLAPPED*/,
                Kernel32.SafeObjectHandle.Null
            );

            if (DeviceHandle.IsInvalid)
                throw new ArgumentException($"Couldn't open device {DevicePath}");

            var data = new AIRBENDER_GET_HOST_BD_ADDR();
            var pData = IntPtr.Zero;
            var bytesReturned = 0;

            Marshal.StructureToPtr(data, pData, false);

            Kernel32.DeviceIoControl(
                DeviceHandle,
                (int)IOCTL_AIRBENDER_GET_HOST_BD_ADDR,
                IntPtr.Zero, 
                0, 
                pData, 
                Marshal.SizeOf(data), 
                out bytesReturned, 
                IntPtr.Zero);
        }

        ~AirBender()
        {
            DeviceHandle.Close();
        }

        public string DevicePath { get; }

        private Kernel32.SafeObjectHandle DeviceHandle { get; }

        public PhysicalAddress HostAddress { get; }
    }
}
