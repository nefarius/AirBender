using System;
using System.Linq;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using PInvoke;
using Serilog;
using SokkaServer.Exceptions;
using SokkaServer.Properties;

namespace SokkaServer
{
    internal partial class AirBender
    {
        private readonly Kernel32.SafeObjectHandle _deviceHandle;

        public AirBender(string devicePath)
        {
            DevicePath = devicePath;

            //
            // Open device
            // 
            _deviceHandle = Kernel32.CreateFile(DevicePath,
                Kernel32.ACCESS_MASK.GenericRight.GENERIC_READ | Kernel32.ACCESS_MASK.GenericRight.GENERIC_WRITE,
                Kernel32.FileShare.FILE_SHARE_READ | Kernel32.FileShare.FILE_SHARE_WRITE,
                IntPtr.Zero, Kernel32.CreationDisposition.OPEN_EXISTING,
                Kernel32.CreateFileFlags.FILE_ATTRIBUTE_NORMAL /*| Kernel32.CreateFileFlags.FILE_FLAG_OVERLAPPED*/,
                Kernel32.SafeObjectHandle.Null
            );

            if (_deviceHandle.IsInvalid)
                throw new ArgumentException($"Couldn't open device {DevicePath}");

            var length = Marshal.SizeOf(typeof(AIRBENDER_GET_HOST_BD_ADDR));
            var pData = Marshal.AllocHGlobal(length);
            var bytesReturned = 0;

            //
            // Request host MAC address
            // 
            var ret = Kernel32.DeviceIoControl(
                _deviceHandle,
                unchecked((int)IOCTL_AIRBENDER_GET_HOST_BD_ADDR),
                IntPtr.Zero, 0, pData, length,
                out bytesReturned, IntPtr.Zero);

            if (!ret)
            {
                Marshal.FreeHGlobal(pData);
                throw new InvalidOperationException("IOCTL_AIRBENDER_GET_HOST_BD_ADDR failed");
            }

            HostAddress = new PhysicalAddress(Marshal.PtrToStructure<AIRBENDER_GET_HOST_BD_ADDR>(pData).Host.Address);

            Marshal.FreeHGlobal(pData);

            Log.Information($"Bluetooth Host Address: {HostAddress.AsFriendlyName()}");

#if TEST
            //
            // Request host controller to reset and clean up resources
            // 
            ret = Kernel32.DeviceIoControl(
                _deviceHandle,
                unchecked((int)IOCTL_AIRBENDER_HOST_RESET),
                IntPtr.Zero, 0, IntPtr.Zero, 0,
                out bytesReturned, IntPtr.Zero);

            if (!ret)
                throw new InvalidOperationException("IOCTL_AIRBENDER_HOST_RESET failed");
#endif

            length = Marshal.SizeOf(typeof(AIRBENDER_GET_CLIENT_COUNT));
            pData = Marshal.AllocHGlobal(length);
            bytesReturned = 0;

            //
            // Request host MAC address
            // 
            ret = Kernel32.DeviceIoControl(
                _deviceHandle,
                unchecked((int)IOCTL_AIRBENDER_GET_CLIENT_COUNT),
                IntPtr.Zero, 0, pData, length,
                out bytesReturned, IntPtr.Zero);

            if (!ret)
            {
                Marshal.FreeHGlobal(pData);
                throw new InvalidOperationException("IOCTL_AIRBENDER_GET_CLIENT_COUNT failed");
            }

            Log.Information($"Currently connected devices: {Marshal.PtrToStructure<AIRBENDER_GET_CLIENT_COUNT>(pData).Count}");

            GetHidInputBufferSize();
        }

        private uint GetHidInputBufferSize(uint clientIndex = 0)
        {
            int bytesReturned;
            uint requiredSize = 0;
            var requestSize = Marshal.SizeOf<AIRBENDER_GET_CLIENT_DETAILS>();
            var requestBuffer = Marshal.AllocHGlobal(requestSize);

            Marshal.StructureToPtr(
                new AIRBENDER_GET_CLIENT_DETAILS()
                {
                    ClientIndex = clientIndex
                },
                requestBuffer, false);

            var ret = Kernel32.DeviceIoControl(
                _deviceHandle,
                unchecked((int)IOCTL_AIRBENDER_GET_CLIENT_STATE),
                requestBuffer, requestSize, requestBuffer, requestSize,
                out bytesReturned, IntPtr.Zero);

            if (!ret && Marshal.GetLastWin32Error() == ERROR_DEV_NOT_EXIST)
            {
                Marshal.FreeHGlobal(requestBuffer);

                throw new AirBenderDeviceNotFoundException();
            }

            if (ret /*&& Marshal.GetLastWin32Error() == ERROR_INSUFFICIENT_BUFFER*/)
            {
                var resp = Marshal.PtrToStructure<AIRBENDER_GET_CLIENT_DETAILS>(requestBuffer);
                
                var client = new PhysicalAddress(resp.ClientAddress.Address.Reverse().ToArray());
            }

            Marshal.FreeHGlobal(requestBuffer);

            return requiredSize;
        }

        public static Guid ClassGuid => Guid.Parse(Settings.Default.ClassGuid);

        public string DevicePath { get; }

        public PhysicalAddress HostAddress { get; }

        ~AirBender()
        {
            _deviceHandle?.Close();
        }
    }
}