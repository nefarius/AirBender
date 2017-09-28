using Serilog;
using Topshelf;

namespace SokkaServer
{
    class Program
    {
        static void Main(string[] args)
        {
            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Information()
                .WriteTo.Console()
                .WriteTo.RollingFile("SokkaServer-{Date}.log")
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
                x.SetDisplayName("SokkaServer");                       
                x.SetServiceName("SokkaServer");                                
            });
        }
    }
}
