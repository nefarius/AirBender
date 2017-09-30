using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.IO;
using System.Reflection;
using AirBender.Common.Shared.Core;
using AirBender.Common.Shared.Plugins;

namespace SokkaServer.Plugins
{
    class PluginHost : IAirBenderSink
    {
        [ImportMany]
        public Lazy<IAirBenderSink, IDictionary<string, object>>[] SinkPlugins { get; set; }

        public PluginHost()
        {
            try
            {
                //Creating an instance of aggregate catalog. It aggregates other catalogs
                var aggregateCatalog = new AggregateCatalog();

                //Build the directory path where the parts will be available
                var directoryPath =
                    Path.Combine(Path.GetDirectoryName
                        (Assembly.GetExecutingAssembly().Location), "Plugins");
                    
                //Load parts from the available DLLs in the specified path 
                //using the directory catalog
                var directoryCatalog = new DirectoryCatalog(directoryPath, "*.dll");

                //Load parts from the current assembly if available
                var asmCatalog = new AssemblyCatalog(Assembly.GetExecutingAssembly());

                //Add to the aggregate catalog
                aggregateCatalog.Catalogs.Add(directoryCatalog);
                aggregateCatalog.Catalogs.Add(asmCatalog);

                //Crete the composition container
                var container = new CompositionContainer(aggregateCatalog);

                // Composable parts are created here i.e. 
                // the Import and Export components assembles here
                container.ComposeParts(this);
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        public void DeviceArrived(ChildDeviceState device)
        {
            foreach (var plugin in SinkPlugins)
            {
                plugin.Value.DeviceArrived(device);
            }
        }

        public void DeviceRemoved(ChildDeviceState device)
        {
            foreach (var plugin in SinkPlugins)
            {
                plugin.Value.DeviceRemoved(device);
            }
        }

        public void InputReportReceived(ChildDeviceState device)
        {
            foreach (var plugin in SinkPlugins)
            {
                plugin.Value.InputReportReceived(device);
            }
        }
    }
}
