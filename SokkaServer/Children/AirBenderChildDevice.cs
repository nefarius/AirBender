using System;
using System.Net.NetworkInformation;
using System.Reactive.Linq;
using System.Threading;
using System.Threading.Tasks;
using SokkaServer.Host;

namespace SokkaServer.Children
{
    public delegate void ChildDeviceDisconnectedEventHandler(object sender, EventArgs e);

    internal abstract class AirBenderChildDevice
    {
        private readonly CancellationTokenSource _inputCancellationTokenSourcePrimary = new CancellationTokenSource();
        private readonly CancellationTokenSource _inputCancellationTokenSourceSecondary = new CancellationTokenSource();
        private readonly IObservable<long> _outputReportSchedule = Observable.Interval(TimeSpan.FromMilliseconds(10));
        private readonly IDisposable _outputReportTask;

        protected AirBenderChildDevice(AirBenderHost host, PhysicalAddress client, int index)
        {
            HostDevice = host;
            ClientAddress = client;
            DeviceIndex = index;

            _outputReportTask = _outputReportSchedule.Subscribe(OnOutputReport);

            //
            // Start two tasks requesting input reports in parallel.
            // 
            // While on threads request gets completed, another request can be
            // queued by the other thread. This way no input can get lost because
            // there's always at least one pending request in the driver to get
            // completed. Each thread uses inverted calls for maximum performance.
            // 
            Task.Factory.StartNew(RequestInputReport, _inputCancellationTokenSourcePrimary.Token);
            Task.Factory.StartNew(RequestInputReport, _inputCancellationTokenSourceSecondary.Token);
        }

        public int DeviceIndex { get; }

        public AirBenderHost HostDevice { get; }

        public PhysicalAddress ClientAddress { get; }

        public event ChildDeviceDisconnectedEventHandler ChildDeviceDisconnected;

        ~AirBenderChildDevice()
        {
            _outputReportTask?.Dispose();

            _inputCancellationTokenSourcePrimary.Cancel();
            _inputCancellationTokenSourceSecondary.Cancel();
        }

        protected virtual void RequestInputReport(object cancellationToken)
        {
        }

        protected virtual void OnOutputReport(long l)
        {
        }

        protected virtual void OnChildDeviceDisconnected(EventArgs e)
        {
            _outputReportTask?.Dispose();

            _inputCancellationTokenSourcePrimary.Cancel();
            _inputCancellationTokenSourceSecondary.Cancel();

            ChildDeviceDisconnected?.Invoke(this, e);
        }
    }
}