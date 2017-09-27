using System.IO;
using ProtoBuf;

namespace AirBender.Common.Shared.Serialization
{
    public static class MessageExtensions
    {
        public static byte[] ToBytes<T>(this T message)
        {
            using (var stream = new MemoryStream())
            {
                Serializer.SerializeWithLengthPrefix(stream, message, PrefixStyle.Fixed32BigEndian);

                return stream.ToArray();
            }
        }

        public static T ToObject<T>(byte[] data)
        {
            using (var stream = new MemoryStream(data))
            {
                return Serializer.DeserializeWithLengthPrefix<T>(stream, PrefixStyle.Fixed32BigEndian);
            }
        }
    }
}