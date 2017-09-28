using AirBender.Common.Shared.Core;
using ProtoBuf;

namespace AirBender.Common.Shared.Messages
{
    [ProtoContract(SkipConstructor = true)]
    public class DeviceMetaMessage
    {
        [ProtoMember(1)]
        public int Index { get; set; }

        [ProtoMember(2)]
        public byte[] Address { get; set; }

        [ProtoMember(3)]
        public BthDeviceType DeviceType { get; set; }

        [ProtoMember(4)]
        public byte[] InputReport { get; set; }

        [ProtoMember(5)]
        public byte[] OutputReport { get; set; }

        public override bool Equals(object obj)
        {
            return Equals(obj as DeviceMetaMessage);
        }

        protected bool Equals(DeviceMetaMessage other)
        {
            return Index == other.Index && DeviceType == other.DeviceType;
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = Index;
                hashCode = (hashCode * 397) ^ (int) DeviceType;
                return hashCode;
            }
        }

        public static bool operator ==(DeviceMetaMessage left, DeviceMetaMessage right)
        {
            return Equals(left, right);
        }

        public static bool operator !=(DeviceMetaMessage left, DeviceMetaMessage right)
        {
            return !Equals(left, right);
        }
    }
}
