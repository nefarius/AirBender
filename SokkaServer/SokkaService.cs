using System;
using Nefarius.Devcon;
using Serilog;

namespace SokkaServer
{
    class SokkaService
    {
        private AirBender BthHost { get; set; }

        public void Start()
        {
            Log.Information("SokkaService started");

            string path = string.Empty, instance = string.Empty;

            try
            {
                if (Devcon.Find(AirBender.ClassGuid, ref path, ref instance))
                {
                    Log.Information($"Found AirBender device {path} ({instance})");

                    BthHost = new AirBender(path);
                }
            }
            catch (Exception ex)
            {
                Log.Fatal($"Unexpected error: {ex}");
            }
        }

        public void Stop()
        {
            BthHost.Dispose();
        }
    }
}
