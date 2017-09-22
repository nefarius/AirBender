using System;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using PInvoke;
using Serilog;
using SokkaServer.Properties;

namespace SokkaServer
{
    internal partial class AirBender
    {
        private readonly Kernel32.SafeObjectHandle _deviceHandle;

        public AirBender(string devicePath)
        {
            DevicePath = devicePath;

            _deviceHandle = Kernel32.CreateFile(DevicePath,
                Kernel32.ACCESS_MASK.GenericRight.GENERIC_READ | Kernel32.ACCESS_MASK.GenericRight.GENERIC_WRITE,
                Kernel32.FileShare.FILE_SHARE_READ | Kernel32.FileShare.FILE_SHARE_WRITE,
                IntPtr.Zero, Kernel32.CreationDisposition.OPEN_EXISTING,
                Kernel32.CreateFileFlags.FILE_ATTRIBUTE_NORMAL /*| Kernel32.CreateFileFlags.FILE_FLAG_OVERLAPPED*/,
                Kernel32.SafeObjectHandle.Null
            );

            if (_deviceHandle.IsInvalid)
                throw new ArgumentException($"Couldn't open device {DevicePath}");

            var length = Marshal.SizeOf(typeof(BD_ADDR));
            var pData = Marshal.AllocHGlobal(length);
            var bytesReturned = 0;

            var ret = Kernel32.DeviceIoControl(
                _deviceHandle,
                unchecked((int) IOCTL_AIRBENDER_GET_HOST_BD_ADDR),
                IntPtr.Zero, 0, pData, length,
                out bytesReturned, IntPtr.Zero);

            HostAddress = new PhysicalAddress(Marshal.PtrToStructure<AIRBENDER_GET_HOST_BD_ADDR>(pData).Host.Address);

            Log.Information($"Bluetooth Host Address: {HostAddress.AsFriendlyName()}");
        }

        public static Guid ClassGuid => Guid.Parse(Settings.Default.ClassGuid);

        public string DevicePath { get; }

        public PhysicalAddress HostAddress { get; }

        ~AirBender()
        {
            _deviceHandle.Close();
        }
    }
}