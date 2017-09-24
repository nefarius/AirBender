using System;
using System.Runtime.Serialization;

namespace SokkaServer.Exceptions
{
    public class AirBenderDeviceNotFoundException : Exception
    {
        public AirBenderDeviceNotFoundException()
        {
        }

        public AirBenderDeviceNotFoundException(string message) : base(message)
        {
        }

        public AirBenderDeviceNotFoundException(string message, Exception innerException) : base(message, innerException)
        {
        }

        protected AirBenderDeviceNotFoundException(SerializationInfo info, StreamingContext context) : base(info, context)
        {
        }
    }
}
