using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;
using System.Reactive.Linq;
using System.Runtime.InteropServices;
using PInvoke;
using Serilog;
using SokkaServer.Children;
using SokkaServer.Exceptions;
using SokkaServer.Properties;

namespace SokkaServer
{
    internal partial class AirBender
    {
        private readonly IObservable<long> _deviceLookupSchedule = Observable.Interval(TimeSpan.FromSeconds(2));
        private readonly IDisposable _deviceLookupTask;

        public AirBender(string devicePath)
        {
            Children = new List<AirBenderChildDevice>();
            DevicePath = devicePath;

            //
            // Open device
            // 
            DeviceHandle = Kernel32.CreateFile(DevicePath,
                Kernel32.ACCESS_MASK.GenericRight.GENERIC_READ | Kernel32.ACCESS_MASK.GenericRight.GENERIC_WRITE,
                Kernel32.FileShare.FILE_SHARE_READ | Kernel32.FileShare.FILE_SHARE_WRITE,
                IntPtr.Zero, Kernel32.CreationDisposition.OPEN_EXISTING,
                Kernel32.CreateFileFlags.FILE_ATTRIBUTE_NORMAL /*| Kernel32.CreateFileFlags.FILE_FLAG_OVERLAPPED*/,
                Kernel32.SafeObjectHandle.Null
            );

            if (DeviceHandle.IsInvalid)
                throw new ArgumentException($"Couldn't open device {DevicePath}");

            var length = Marshal.SizeOf(typeof(AIRBENDER_GET_HOST_BD_ADDR));
            var pData = Marshal.AllocHGlobal(length);
            var bytesReturned = 0;
            bool ret;

            try
            {
                //
                // Request host MAC address
                // 
                ret = Kernel32.DeviceIoControl(
                    DeviceHandle,
                    unchecked((int) IOCTL_AIRBENDER_GET_HOST_BD_ADDR),
                    IntPtr.Zero, 0, pData, length,
                    out bytesReturned, IntPtr.Zero);

                if (!ret)
                    throw new InvalidOperationException("IOCTL_AIRBENDER_GET_HOST_BD_ADDR failed");

                HostAddress =
                    new PhysicalAddress(Marshal.PtrToStructure<AIRBENDER_GET_HOST_BD_ADDR>(pData).Host.Address);
            }
            finally
            {
                Marshal.FreeHGlobal(pData);
            }

            Log.Information($"Bluetooth Host Address: {HostAddress.AsFriendlyName()}");

            //
            // Request host controller to reset and clean up resources
            // 
            ret = Kernel32.DeviceIoControl(
                DeviceHandle,
                unchecked((int) IOCTL_AIRBENDER_HOST_RESET),
                IntPtr.Zero, 0, IntPtr.Zero, 0,
                out bytesReturned, IntPtr.Zero);

            if (!ret)
                throw new AirBenderHostResetFailedException();

            _deviceLookupTask = _deviceLookupSchedule.Subscribe(OnLookup);
        }

        public Kernel32.SafeObjectHandle DeviceHandle { get; }

        private List<AirBenderChildDevice> Children { get; }

        public static Guid ClassGuid => Guid.Parse(Settings.Default.ClassGuid);

        public string DevicePath { get; }

        public PhysicalAddress HostAddress { get; }

        private void OnLookup(long l)
        {
            var length = Marshal.SizeOf(typeof(AIRBENDER_GET_CLIENT_COUNT));
            var pData = Marshal.AllocHGlobal(length);
            var bytesReturned = 0;

            //
            // Request client count
            // 
            var ret = Kernel32.DeviceIoControl(
                DeviceHandle,
                unchecked((int) IOCTL_AIRBENDER_GET_CLIENT_COUNT),
                IntPtr.Zero, 0, pData, length,
                out bytesReturned, IntPtr.Zero);

            if (!ret)
            {
                Marshal.FreeHGlobal(pData);
                throw new AirbenderGetClientCountFailedException();
            }

            var count = Marshal.PtrToStructure<AIRBENDER_GET_CLIENT_COUNT>(pData).Count;

            //
            // Return if no children or all children are already known
            // 
            if (count == 0 || count == Children.Count) return;

            Log.Information($"Currently connected devices: {count}");

            for (uint i = 0; i < count; i++)
            {
                PhysicalAddress address;
                BTH_DEVICE_TYPE type;

                if (!GetDeviceStateByIndex(i, out address, out type))
                    continue;

                switch (type)
                {
                    case BTH_DEVICE_TYPE.DualShock3:
                        var device = new AirBenderDualShock3(this, address);
                        device.ChildDeviceDisconnected += OnChildDeviceDisconnected;
                        Children.Add(device);
                        break;
                    case BTH_DEVICE_TYPE.DualShock4:
                        throw new NotImplementedException();
                        break;
                }
            }
        }

        private void OnChildDeviceDisconnected(object sender, EventArgs eventArgs)
        {
            throw new NotImplementedException();
        }

        private bool GetDeviceStateByIndex(uint clientIndex, out PhysicalAddress address, out BTH_DEVICE_TYPE type)
        {
            int bytesReturned;
            var requestSize = Marshal.SizeOf<AIRBENDER_GET_CLIENT_DETAILS>();
            var requestBuffer = Marshal.AllocHGlobal(requestSize);

            Marshal.StructureToPtr(
                new AIRBENDER_GET_CLIENT_DETAILS
                {
                    ClientIndex = clientIndex
                },
                requestBuffer, false);

            var ret = Kernel32.DeviceIoControl(
                DeviceHandle,
                unchecked((int) IOCTL_AIRBENDER_GET_CLIENT_STATE),
                requestBuffer, requestSize, requestBuffer, requestSize,
                out bytesReturned, IntPtr.Zero);

            if (!ret && Marshal.GetLastWin32Error() == ERROR_DEV_NOT_EXIST)
            {
                Marshal.FreeHGlobal(requestBuffer);

                throw new AirBenderDeviceNotFoundException();
            }

            try
            {
                if (ret /*&& Marshal.GetLastWin32Error() == ERROR_INSUFFICIENT_BUFFER*/)
                {
                    var resp = Marshal.PtrToStructure<AIRBENDER_GET_CLIENT_DETAILS>(requestBuffer);

                    type = resp.DeviceType;
                    address = new PhysicalAddress(resp.ClientAddress.Address.Reverse().ToArray());

                    return true;
                }
            }
            finally
            {
                Marshal.FreeHGlobal(requestBuffer);
            }

            type = BTH_DEVICE_TYPE.Unknown;
            address = PhysicalAddress.None;

            return false;
        }

        ~AirBender()
        {
            _deviceLookupTask?.Dispose();
            DeviceHandle?.Close();
        }
    }
}