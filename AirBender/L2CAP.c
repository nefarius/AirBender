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
