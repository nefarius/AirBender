using System;
using AirBender.Sokka.Server.Host;
using Nefarius.Devcon;
using Serilog;

namespace AirBender.Sokka.Server
{
    internal class SokkaService
    {
        private AirBenderHost BthHost { get; set; }

        public void Start()
        {
            Log.Information("AirBender Sokka Server started");

            string path = string.Empty, instance = string.Empty;

            try
            {
                if (Devcon.Find(AirBenderHost.ClassGuid, ref path, ref instance))
                {
                    Log.Information($"Found AirBender device {path} ({instance})");

                    BthHost = new AirBenderHost(path);
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

            Log.Information("AirBender Sokka Server stopped");
        }
    }
}