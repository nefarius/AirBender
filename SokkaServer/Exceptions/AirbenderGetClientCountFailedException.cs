using System;
using System.Runtime.Serialization;

namespace SokkaServer.Exceptions
{
    public class AirbenderGetClientCountFailedException : Exception
    {
        public AirbenderGetClientCountFailedException()
        {
        }

        public AirbenderGetClientCountFailedException(string message) : base(message)
        {
        }

        public AirbenderGetClientCountFailedException(string message, Exception innerException) : base(message, innerException)
        {
        }

        protected AirbenderGetClientCountFailedException(SerializationInfo info, StreamingContext context) : base(info, context)
        {
        }
    }
}
