using System;
using PInvoke;

namespace SokkaServer
{
    class AirBender
    {
        public static Guid ClassGuid => Guid.Parse("a775e97e-a41b-4bfc-868e-25be84643b62");

        public AirBender(string devicePath)
        {
            DevicePath = devicePath;

            DeviceHandle = Kernel32.CreateFile(DevicePath,
                Kernel32.ACCESS_MASK.GenericRight.GENERIC_READ | Kernel32.ACCESS_MASK.GenericRight.GENERIC_WRITE,
                Kernel32.FileShare.FILE_SHARE_READ | Kernel32.FileShare.FILE_SHARE_WRITE,
                IntPtr.Zero, Kernel32.CreationDisposition.OPEN_EXISTING,
                Kernel32.CreateFileFlags.FILE_ATTRIBUTE_NORMAL | Kernel32.CreateFileFlags.FILE_FLAG_OVERLAPPED,
                Kernel32.SafeObjectHandle.Null
            );

            if (DeviceHandle.IsInvalid)
                throw new ArgumentException($"Couldn't open device {DevicePath}");
        }

        ~AirBender()
        {
            DeviceHandle.Close();
        }

        public string DevicePath { get; }

        private Kernel32.SafeObjectHandle DeviceHandle { get; }
    }
}
