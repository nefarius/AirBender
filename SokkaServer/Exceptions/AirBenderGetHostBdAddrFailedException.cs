using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace SokkaServer.Exceptions
{
    public class AirBenderGetHostBdAddrFailedException : Exception
    {
        public AirBenderGetHostBdAddrFailedException()
        {
        }

        public AirBenderGetHostBdAddrFailedException(string message) : base(message)
        {
        }

        public AirBenderGetHostBdAddrFailedException(string message, Exception innerException) : base(message, innerException)
        {
        }

        protected AirBenderGetHostBdAddrFailedException(SerializationInfo info, StreamingContext context) : base(info, context)
        {
        }
    }
}
