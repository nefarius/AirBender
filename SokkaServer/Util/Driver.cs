using System;
using System.Runtime.InteropServices;
using System.Threading;
using PInvoke;

namespace SokkaServer.Util
{
    internal class Driver
    {
        public static bool OverlappedDeviceIoControl(Kernel32.SafeObjectHandle handle, uint ioControlCode,
            IntPtr inBuffer, int inBufferSize, IntPtr outBuffer, int outBufferSize, out int bytesReturned)
        {
            var resetEvent = new ManualResetEvent(false);
            var overlapped = Marshal.AllocHGlobal(Marshal.SizeOf<NativeOverlapped>());
            Marshal.StructureToPtr(new NativeOverlapped {EventHandle = resetEvent.SafeWaitHandle.DangerousGetHandle()},
                overlapped, false);

            try
            {
                Kernel32.DeviceIoControl(
                    handle,
                    unchecked((int) ioControlCode),
                    inBuffer, inBufferSize, outBuffer, outBufferSize,
                    out bytesReturned, overlapped);

                return Kernel32.GetOverlappedResult(handle, overlapped, out bytesReturned, true);
            }
            finally
            {
                resetEvent.Dispose();
                Marshal.FreeHGlobal(overlapped);
            }
        }
    }
}