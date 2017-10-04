using System;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Net.NetworkInformation;
using System.Reactive.Linq;
using System.Runtime.InteropServices;
using AirBender.Common.Shared.Core;
using AirBender.Sokka.Server.Children;
using AirBender.Sokka.Server.Children.DualShock3;
using AirBender.Sokka.Server.Exceptions;
using AirBender.Sokka.Server.Plugins;
using AirBender.Sokka.Server.Properties;
using AirBender.Sokka.Server.Util;
using PInvoke;
using Serilog;

namespace AirBender.Sokka.Server.Host
{
    internal partial class AirBenderHost : IDisposable
    {
        private readonly IObservable<long> _deviceLookupSchedule = Observable.Interval(TimeSpan.FromSeconds(2));
        private readonly IDisposable _deviceLookupTask;
        private readonly PluginHost _plugins = new PluginHost();

        public AirBenderHost(string devicePath)
        {
            DevicePath = devicePath;
            Children = new ObservableCollection<AirBenderChildDevice>();

            Children.CollectionChanged += (sender, args) =>
            {
                switch (args.Action)
                {
                    case NotifyCollectionChangedAction.Add:

                        foreach (IAirBenderChildDevice item in args.NewItems)
                            _plugins.DeviceArrived(item);

                        break;
                    case NotifyCollectionChangedAction.Remove:

                        foreach (IAirBenderChildDevice item in args.OldItems)
                            _plugins.DeviceRemoved(item);

                        break;
                    default:
                        break;
                }
            };

            //
            // Open device
            // 
            DeviceHandle = Kernel32.CreateFile(DevicePath,
                Kernel32.ACCESS_MASK.GenericRight.GENERIC_READ | Kernel32.ACCESS_MASK.GenericRight.GENERIC_WRITE,
                Kernel32.FileShare.FILE_SHARE_READ | Kernel32.FileShare.FILE_SHARE_WRITE,
                IntPtr.Zero, Kernel32.CreationDisposition.OPEN_EXISTING,
                Kernel32.CreateFileFlags.FILE_ATTRIBUTE_NORMAL | Kernel32.CreateFileFlags.FILE_FLAG_OVERLAPPED,
                Kernel32.SafeObjectHandle.Null
            );

            if (DeviceHandle.IsInvalid)
                throw new ArgumentException($"Couldn't open device {DevicePath}");

            var length = Marshal.SizeOf(typeof(AirBenderHost.AirbenderGetHostBdAddr));
            var pData = Marshal.AllocHGlobal(length);
            var bytesReturned = 0;
            bool ret;

            try
            {
                //
                // Request host MAC address
                // 
                ret = DeviceHandle.OverlappedDeviceIoControl(
                    AirBenderHost.IoctlAirbenderGetHostBdAddr,
                    IntPtr.Zero, 0, pData, length,
                    out bytesReturned);

                if (!ret)
                    throw new AirBenderGetHostBdAddrFailedException();

                HostAddress =
                    new PhysicalAddress(Marshal.PtrToStructure<AirBenderHost.AirbenderGetHostBdAddr>(pData).Host.Address.Reverse()
                        .ToArray());
            }
            finally
            {
                Marshal.FreeHGlobal(pData);
            }

            Log.Information($"Bluetooth Host Address: {HostAddress.AsFriendlyName()}");

            //
            // Request host controller to reset and clean up resources
            // 
            ret = DeviceHandle.OverlappedDeviceIoControl(
                AirBenderHost.IoctlAirbenderHostReset,
                IntPtr.Zero, 0, IntPtr.Zero, 0,
                out bytesReturned);

            if (!ret)
                throw new AirBenderHostResetFailedException();

            _deviceLookupTask = _deviceLookupSchedule.Subscribe(OnLookup);
        }

        public Kernel32.SafeObjectHandle DeviceHandle { get; }

        private ObservableCollection<AirBenderChildDevice> Children { get; }

        public static Guid ClassGuid => Guid.Parse(Settings.Default.ClassGuid);

        public string DevicePath { get; }

        public PhysicalAddress HostAddress { get; }

        private void OnLookup(long l)
        {
            var length = Marshal.SizeOf(typeof(AirBenderHost.AirbenderGetClientCount));
            var pData = Marshal.AllocHGlobal(length);

            try
            {
                //
                // Request client count
                // 
                var bytesReturned = 0;
                var ret = DeviceHandle.OverlappedDeviceIoControl(
                    AirBenderHost.IoctlAirbenderGetClientCount,
                    IntPtr.Zero, 0, pData, length,
                    out bytesReturned);

                //
                // This happens if the host device got "surprise-removed"
                // 
                if (!ret && Marshal.GetLastWin32Error() == AirBenderHost.ErrorBadCommand)
                {
                    Log.Warning($"Connection to device {DevicePath} lost, possibly it got removed");

                    Dispose();
                    return;
                }

                //
                // Here something terrible happened
                // 
                if (!ret)
                {
                    Log.Error($"Unexpected error: {new Win32Exception(Marshal.GetLastWin32Error())}");
                    return;
                }

                var count = Marshal.PtrToStructure<AirBenderHost.AirbenderGetClientCount>(pData).Count;

                //
                // Return if no children or all children are already known
                // 
                if (count == 0 || count == Children.Count) return;

                Log.Information($"Currently connected devices: {count}");

                for (uint i = 0; i < count; i++)
                {
                    PhysicalAddress address;
                    BthDeviceType type;

                    if (!GetDeviceStateByIndex(i, out address, out type))
                        continue;

                    switch (type)
                    {
                        case BthDeviceType.DualShock3:
                            var device = new AirBenderDualShock3(this, address, (int) i);

                            device.ChildDeviceDisconnected +=
                                (sender, args) => Children.Remove((AirBenderChildDevice) sender);
                            device.InputReportReceived +=
                                (sender, args) => _plugins.InputReportReceived((AirBenderChildDevice) sender,
                                    args.Report);

                            Children.Add(device);

                            break;
                        case BthDeviceType.DualShock4:
                            throw new NotImplementedException();
                    }
                }
            }
            finally
            {
                Marshal.FreeHGlobal(pData);
            }
        }

        private bool GetDeviceStateByIndex(uint clientIndex, out PhysicalAddress address, out BthDeviceType type)
        {
            var requestSize = Marshal.SizeOf<AirBenderHost.AirbenderGetClientDetails>();
            var requestBuffer = Marshal.AllocHGlobal(requestSize);

            try
            {
                Marshal.StructureToPtr(
                    new AirBenderHost.AirbenderGetClientDetails
                    {
                        ClientIndex = clientIndex
                    },
                    requestBuffer, false);

                int bytesReturned;
                var ret = DeviceHandle.OverlappedDeviceIoControl(
                    AirBenderHost.IoctlAirbenderGetClientState,
                    requestBuffer, requestSize, requestBuffer, requestSize,
                    out bytesReturned);

                if (!ret && Marshal.GetLastWin32Error() == AirBenderHost.ErrorDevNotExist)
                    throw new AirBenderDeviceNotFoundException();

                if (ret)
                {
                    var resp = Marshal.PtrToStructure<AirBenderHost.AirbenderGetClientDetails>(requestBuffer);

                    type = resp.DeviceType;
                    address = new PhysicalAddress(resp.ClientAddress.Address.Reverse().ToArray());

                    return true;
                }
            }
            finally
            {
                Marshal.FreeHGlobal(requestBuffer);
            }

            type = BthDeviceType.Unknown;
            address = PhysicalAddress.None;

            return false;
        }

        #region IDisposable Support

        private bool disposedValue; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    _deviceLookupTask?.Dispose();
                    Children.Clear();
                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.
                int bytesReturned;
                DeviceHandle.OverlappedDeviceIoControl(
                    AirBenderHost.IoctlAirbenderHostShutdown,
                    IntPtr.Zero, 0, IntPtr.Zero, 0, out bytesReturned);

                DeviceHandle?.Close();

                disposedValue = true;
            }
        }

        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
        ~AirBenderHost()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(false);
        }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
            // TODO: uncomment the following line if the finalizer is overridden above.
            GC.SuppressFinalize(this);
        }

        #endregion
    }
}