using System;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using PInvoke;
using Serilog;

namespace SokkaServer
{
    internal partial class AirBender
    {
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

            var length = Marshal.SizeOf(typeof(BD_ADDR));
            var pData = Marshal.AllocHGlobal(length);
            var bytesReturned = 0;

            var ret = Kernel32.DeviceIoControl(
                DeviceHandle,
                unchecked((int) IOCTL_AIRBENDER_GET_HOST_BD_ADDR),
                IntPtr.Zero, 0, pData, length,
                out bytesReturned, IntPtr.Zero);

            HostAddress = new PhysicalAddress(Marshal.PtrToStructure<AIRBENDER_GET_HOST_BD_ADDR>(pData).Host.Address);

            Log.Information($"Bluetooth Host Address: {HostAddress.AsFriendlyName()}");
        }
        
        public static Guid ClassGuid => Guid.Parse("a775e97e-a41b-4bfc-868e-25be84643b62");

        public string DevicePath { get; }

        private Kernel32.SafeObjectHandle DeviceHandle { get; }

        public PhysicalAddress HostAddress { get; }

        ~AirBender()
        {
            DeviceHandle.Close();
        }
    }
}