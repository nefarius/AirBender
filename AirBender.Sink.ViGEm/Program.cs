using Serilog;
using Topshelf;

namespace AirBender.Sink.ViGEm
{
    class Program
    {
        static void Main(string[] args)
        {
            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Information()
                .WriteTo.Console()
                .WriteTo.RollingFile("AirBenderViGEmSink-{Date}.log")
                .CreateLogger();

            HostFactory.Run(x =>
            {
                x.Service<ViGEmSink>(s =>
                {
                    s.ConstructUsing(name => new ViGEmSink());
                    s.WhenStarted(tc => tc.Start());
                    s.WhenStopped(tc => tc.Stop());
                });
                x.RunAsLocalSystem();

                x.SetDescription("Communicates with AirBender Bluetooth Host Devices.");
                x.SetDisplayName("AirBenderViGEmSink");
                x.SetServiceName("AirBenderViGEmSink");
            });
        }
    }
}
