/*
MIT License

Copyright (c) 2017 Benjamin "Nefarius" Höglinger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "Driver.h"
#include "L2CAP.h"

NTSTATUS
L2CAP_Command(
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

    return WriteBulkPipe(Context, buffer, BufferLength + 8, NULL);
}

NTSTATUS
L2CAP_Command_Connection_Request(
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

NTSTATUS
L2CAP_Command_Connection_Response(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID DestinationChannelId,
    L2CAP_CID SourceChannelId,
    L2CAP_CONNECTION_RESPONSE_RESULT Result,
    L2CAP_CONNECTION_RESPONSE_STATUS Status)
{
    BYTE buffer[12];

    buffer[0] = 0x03;
    buffer[1] = Id;
    buffer[2] = 0x08;
    buffer[3] = 0x00;
    buffer[4] = SourceChannelId.Lsb;
    buffer[5] = SourceChannelId.Msb;
    buffer[6] = DestinationChannelId.Lsb;
    buffer[7] = DestinationChannelId.Msb;
    buffer[8] = (BYTE)Result;
    buffer[9] = 0x00;

    if (Result == L2CAP_ConnectionResponseResult_ConnectionPending)
    {
        buffer[10] = (BYTE)Status;
        buffer[11] = 0x00;
    }
    else
    {
        buffer[10] = 0x00;
        buffer[11] = 0x00;
    }

    return L2CAP_Command(Context, Handle, buffer, 12);
}

NTSTATUS
L2CAP_Command_Configuration_Request(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID DestinationChannelId,
    BOOLEAN SetMtu)
{
    BYTE buffer[12];

    buffer[0] = 0x04;
    buffer[1] = Id;
    buffer[2] = (BYTE)(SetMtu ? 0x08 : 0x04);
    buffer[3] = 0x00;
    buffer[4] = DestinationChannelId.Lsb;
    buffer[5] = DestinationChannelId.Msb;
    buffer[6] = 0x00;
    buffer[7] = 0x00;

    if (SetMtu)
    {
        buffer[8] = 0x01;
        buffer[9] = 0x02;
        buffer[10] = 0x96;
        buffer[11] = 0x00;
    }

    return L2CAP_Command(Context, Handle, buffer, SetMtu ? 12 : 8);
}

NTSTATUS
L2CAP_Command_Configuration_Response(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID SourceChannelId)
{
    BYTE buffer[10];

    buffer[0] = 0x05;
    buffer[1] = Id;
    buffer[2] = 0x06;
    buffer[3] = 0x00;
    buffer[4] = SourceChannelId.Lsb;
    buffer[5] = SourceChannelId.Msb;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x00;

    return L2CAP_Command(Context, Handle, buffer, 10);
}

NTSTATUS
L2CAP_Command_Disconnection_Request(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID DestinationChannelId,
    L2CAP_CID SourceChannelId)
{
    BYTE buffer[8];

    buffer[0] = 0x06;
    buffer[1] = Id;
    buffer[2] = 0x04;
    buffer[3] = 0x00;
    buffer[4] = DestinationChannelId.Lsb;
    buffer[5] = DestinationChannelId.Msb;
    buffer[6] = SourceChannelId.Lsb;
    buffer[7] = SourceChannelId.Msb;

    return L2CAP_Command(Context, Handle, buffer, 8);
}

NTSTATUS
L2CAP_Command_Disconnection_Response(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID DestinationChannelId,
    L2CAP_CID SourceChannelId)
{
    BYTE buffer[8];

    buffer[0] = 0x07;
    buffer[1] = Id;
    buffer[2] = 0x04;
    buffer[3] = 0x00;
    buffer[4] = DestinationChannelId.Lsb;
    buffer[5] = DestinationChannelId.Msb;
    buffer[6] = SourceChannelId.Lsb;
    buffer[7] = SourceChannelId.Msb;

    return L2CAP_Command(Context, Handle, buffer, 8);
}
