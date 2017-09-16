#include "Driver.h"
#include "L2CAP.h"

NTSTATUS L2CAP_Command(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    PVOID Buffer,
    ULONG BufferLength)
{
    BYTE buffer[64];

    buffer[0] = Handle.Lsb;
    buffer[1] = Handle.Msb | 0x20;
    buffer[2] = (BYTE)BufferLength + 4;
    buffer[3] = 0x00;
    buffer[4] = (BYTE)BufferLength;
    buffer[5] = 0x00;
    buffer[6] = 0x01;
    buffer[7] = 0x00;

    RtlCopyMemory(&buffer[8], Buffer, BufferLength);

    return WriteBulkPipe(Context, buffer, 64, NULL);
}

NTSTATUS L2CAP_Command_Connection_Request(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id, 
    L2CAP_CID DestinationChannelId, 
    L2CAP_PSM ProtocolServiceMultiplexer)
{
    BYTE buffer[8];

    buffer[0] = 0x02;
    buffer[1] = Id;
    buffer[2] = 0x04;
    buffer[3] = 0x00;
    buffer[4] = (BYTE)ProtocolServiceMultiplexer;
    buffer[5] = 0x00;
    buffer[6] = DestinationChannelId.Lsb;
    buffer[7] = DestinationChannelId.Msb;

    return L2CAP_Command(Context, Handle, buffer, 8);
}
