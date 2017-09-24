using System;
using System.Net.NetworkInformation;
using System.Reactive.Linq;

namespace SokkaServer
{
    public delegate void ChildDeviceDisconnectedEventHandler(object sender, EventArgs e);

    internal abstract class AirBenderChildDevice
    {
        private readonly IObservable<long> _inputReportSchedule = Observable.Interval(TimeSpan.FromMilliseconds(4));
        private readonly IDisposable _inputReportTask;
        private readonly IObservable<long> _outputReportSchedule = Observable.Interval(TimeSpan.FromMilliseconds(10));
        private readonly IDisposable _outputReportTask;

        public event ChildDeviceDisconnectedEventHandler ChildDeviceDisconnected;

        protected AirBenderChildDevice(AirBender host, PhysicalAddress client)
        {
            HostDevice = host;
            ClientAddress = client;

            _outputReportTask = _outputReportSchedule.Subscribe(OnOutputReport);
            _inputReportTask = _inputReportSchedule.Subscribe(OnInputReport);
        }

        ~AirBenderChildDevice()
        {
            _outputReportTask?.Dispose();
            _inputReportTask?.Dispose();
        }

        public AirBender HostDevice { get; }

        public PhysicalAddress ClientAddress { get; }

        protected virtual void OnInputReport(long l)
        {
        }

        protected virtual void OnOutputReport(long l)
        {
        }

        protected virtual void OnChildDeviceDisconnected(EventArgs e)
        {
            _outputReportTask?.Dispose();
            _inputReportTask?.Dispose();

            ChildDeviceDisconnected?.Invoke(this, e);
        }
    }
}