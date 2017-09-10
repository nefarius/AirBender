#include "Driver.h"

NTSTATUS 
HCI_Command(
    PDEVICE_CONTEXT Context, 
    HCI_COMMAND Command, 
    PVOID Buffer, 
    ULONG BufferLength)
{
    ((PUCHAR)Buffer)[0] = (BYTE)(((ULONG)Command >> 0) & 0xFF);
    ((PUCHAR)Buffer)[1] = (BYTE)(((ULONG)Command >> 8) & 0xFF);
    ((PUCHAR)Buffer)[2] = (BYTE)(BufferLength - 3);

    return SendControlRequest(Context, 
        BmRequestClass, 
        0x0000, 
        0, 
        0, 
        Buffer, 
        BufferLength);
}

NTSTATUS
HCI_Command_Reset(
    PDEVICE_CONTEXT Context)
{
    UCHAR buffer[3];

    return HCI_Command(Context, HCI_Reset, buffer, 3);
}
