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
    }
}
