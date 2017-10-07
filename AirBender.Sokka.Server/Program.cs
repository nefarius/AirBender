using Serilog;
using Topshelf;

namespace AirBender.Sokka.Server
{
    class Program
    {
        static void Main(string[] args)
        {
            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Information()
                .WriteTo.Console()
                .WriteTo.RollingFile("AirBender.Sokka.Server-{Date}.log")
                .CreateLogger();

            HostFactory.Run(x =>                                 
            {
                x.Service<SokkaService>(s =>                     
                {
                    s.ConstructUsing(name => new SokkaService());
                    s.WhenStarted(tc => tc.Start());             
                    s.WhenStopped(tc => tc.Stop());              
                });
                x.RunAsLocalSystem();                            

                x.SetDescription("Communicates with AirBender Bluetooth Host Devices.");        
                x.SetDisplayName("AirBender Sokka Server");                       
                x.SetServiceName("AirBender.Sokka.Server");                                
            });
        }
    }
}
