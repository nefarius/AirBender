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
    BYTE buffer[3];

    return HCI_Command(Context, HCI_Reset, buffer, 3);
}

NTSTATUS
HCI_Command_Accept_Connection_Request(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr,
    BYTE role)
{
    BYTE buffer[10];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    buffer[9] = role;

    return HCI_Command(Context, HCI_Accept_Connection_Request, buffer, 10);
}

NTSTATUS
HCI_Command_Reject_Connection_Request(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr,
    BYTE reason)
{
    BYTE buffer[10];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    buffer[9] = reason;

    return HCI_Command(Context, HCI_Reject_Connection_Request, buffer, 10);
}

NTSTATUS
HCI_Command_Remote_Name_Request(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr)
{
    BYTE buffer[13];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    buffer[9] = 0x01;
    buffer[10] = 0x00;
    buffer[11] = 0x00;
    buffer[12] = 0x00;

    return HCI_Command(Context, HCI_Remote_Name_Request, buffer, 13);
}

NTSTATUS
HCI_Command_Write_Scan_Enable(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[4];

    buffer[3] = 0x02;

    return HCI_Command(Context, HCI_Write_Scan_Enable, buffer, 4);
}

NTSTATUS
HCI_Command_Read_Local_Version_Info(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[3];

    return HCI_Command(Context, HCI_Read_Local_Version_Info, buffer, 3);
}

NTSTATUS
HCI_Command_Read_BD_Addr(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[3];

    return HCI_Command(Context, HCI_Read_BD_ADDR, buffer, 3);
}

NTSTATUS
HCI_Command_Read_Buffer_Size(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[3];

    return HCI_Command(Context, HCI_Read_Buffer_Size, buffer, 3);
}

NTSTATUS
HCI_Command_Link_Key_Request_Reply(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr)
{
    BYTE buffer[25];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    RtlCopyMemory(&buffer[9], BD_LINK, BD_LINK_LENGTH);

    return HCI_Command(Context, HCI_Link_Key_Request_Reply, buffer, 25);
}

NTSTATUS
HCI_Command_Link_Key_Request_Negative_Reply(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr)
{
    BYTE buffer[9];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    return HCI_Command(Context, HCI_Link_Key_Request_Negative_Reply, buffer, 9);
}

NTSTATUS
HCI_Command_PIN_Code_Request_Negative_Reply(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr)
{
    BYTE buffer[16];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    return HCI_Command(Context, HCI_Link_Key_Request_Negative_Reply, buffer, 16);
}

NTSTATUS
HCI_Command_Set_Connection_Encryption(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle)
{
    BYTE buffer[6];

    buffer[3] = Handle.Lsb;
    buffer[4] = (Handle.Msb ^ 0x20);
    buffer[5] = 0x01;

    return HCI_Command(Context, HCI_Set_Connection_Encryption, buffer, 6);
}

NTSTATUS
HCI_Command_User_Confirmation_Request_Reply(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr)
{
    BYTE buffer[9];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    return HCI_Command(Context, HCI_User_Confirmation_Request_Reply, buffer, 9);
}

NTSTATUS
HCI_Command_IO_Capability_Request_Reply(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr)
{
    BYTE buffer[12];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    buffer[9] = 0x01;
    buffer[10] = 0x00;
    buffer[11] = 0x05;

    return HCI_Command(Context, HCI_IO_Capability_Request_Reply, buffer, 12);
}

NTSTATUS
HCI_Command_Set_Event_Mask(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[11];
    // 00 25 5F FF FF FF FF FF
    buffer[3] = 0xFF;
    buffer[4] = 0xFF;
    buffer[5] = 0xFF;
    buffer[6] = 0xFF;
    buffer[7] = 0xFF;
    buffer[8] = 0x5F; // 0xFF;
    buffer[9] = 0x25; // 0xBF;
    buffer[10] = 0x00; // 0x3D;

    return HCI_Command(Context, HCI_Set_Event_Mask, buffer, 11);
}

NTSTATUS
HCI_Command_Write_Local_Name(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[251];

    buffer[3] = 0x45;
    buffer[4] = 0x4E;
    buffer[5] = 0x54;
    buffer[6] = 0x52;
    buffer[7] = 0x4F;
    buffer[8] = 0x50;
    buffer[9] = 0x59;

    return HCI_Command(Context, HCI_Write_Local_Name, buffer, 251);
}

NTSTATUS
HCI_Command_Write_Extended_Inquiry_Response(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[244];

    buffer[3] = 0x00;
    buffer[4] = 0x08;
    buffer[5] = 0x09;
    buffer[6] = 0x45;
    buffer[7] = 0x4E;
    buffer[8] = 0x54;
    buffer[9] = 0x52;
    buffer[10] = 0x4F;
    buffer[11] = 0x50;
    buffer[12] = 0x59;
    buffer[13] = 0x02;
    buffer[14] = 0x0A;

    return HCI_Command(Context, HCI_Write_Extended_Inquiry_Response, buffer, 244);
}

NTSTATUS
HCI_Command_Write_Class_of_Device(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[6];

    buffer[3] = 0x04;
    buffer[4] = 0x02;
    buffer[5] = 0x3E;

    return HCI_Command(Context, HCI_Write_Class_of_Device, buffer, 6);
}

NTSTATUS
HCI_Command_Write_Inquiry_Scan_Type(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[4];

    buffer[3] = 0x01;

    return HCI_Command(Context, HCI_Write_Inquiry_Scan_Type, buffer, 4);
}

NTSTATUS
HCI_Command_Write_Inquiry_Scan_Activity(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[7];

    buffer[3] = 0x00;
    buffer[4] = 0x08;
    buffer[5] = 0x12;
    buffer[6] = 0x00;

    return HCI_Command(Context, HCI_Write_Inquiry_Scan_Activity, buffer, 7);
}

NTSTATUS
HCI_Command_Write_Page_Scan_Type(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[4];

    buffer[3] = 0x01;

    return HCI_Command(Context, HCI_Write_Page_Scan_Type, buffer, 4);
}

NTSTATUS
HCI_Command_Write_Page_Scan_Activity(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[7];

    buffer[3] = 0x00;
    buffer[4] = 0x04;
    buffer[5] = 0x12;
    buffer[6] = 0x00;

    return HCI_Command(Context, HCI_Write_Page_Scan_Activity, buffer, 7);
}

NTSTATUS
HCI_Command_Write_Page_Timeout(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[5];

    buffer[3] = 0x00;
    buffer[4] = 0x20;

    return HCI_Command(Context, HCI_Write_Page_Timeout, buffer, 5);
}

NTSTATUS
HCI_Command_Write_Authentication_Enable(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[4];

    buffer[3] = 0x00;

    return HCI_Command(Context, HCI_Write_Authentication_Enable, buffer, 4);
}

NTSTATUS
HCI_Command_Write_Simple_Pairing_Mode(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[4];

    buffer[3] = 0x01;

    return HCI_Command(Context, HCI_Write_Simple_Pairing_Mode, buffer, 4);
}

NTSTATUS
HCI_Command_Write_Simple_Pairing_Debug_Mode(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[4];

    buffer[3] = 0x00;

    return HCI_Command(Context, HCI_Write_Simple_Pairing_Debug_Mode, buffer, 4);
}

NTSTATUS
HCI_Command_Write_Inquiry_Mode(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[4];

    buffer[3] = 0x02;

    return HCI_Command(Context, HCI_Write_Inquiry_Mode, buffer, 4);
}

NTSTATUS
HCI_Command_Write_Inquiry_Transmit_Power_Level(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[4];

    buffer[3] = 0x00;

    return HCI_Command(Context, HCI_Write_Inquiry_Transmit_Power_Level, buffer, 4);
}

NTSTATUS HCI_Command_Inquiry(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[8];

    buffer[3] = 0x33;
    buffer[4] = 0x8B;
    buffer[5] = 0x9E;
    buffer[6] = 0x18;
    buffer[7] = 0x00;

    return HCI_Command(Context, HCI_Inquiry, buffer, 8);
}

NTSTATUS HCI_Command_Inquiry_Cancel(
    PDEVICE_CONTEXT Context)
{
    BYTE buffer[3];

    return HCI_Command(Context, HCI_Inquiry_Cancel, buffer, 3);
}

NTSTATUS
HCI_Command_Delete_Stored_Link_Key(
    PDEVICE_CONTEXT Context,
    BD_ADDR BdAddr)
{
    BYTE buffer[10];

    RtlCopyMemory(&buffer[3], &BdAddr, sizeof(BD_ADDR));

    buffer[9] = 0x00;

    return HCI_Command(Context, HCI_Delete_Stored_Link_Key, buffer, 10);
}

NTSTATUS
HCI_Command_Disconnect(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle)
{
    BYTE buffer[6];

    buffer[3] = Handle.Lsb;
    buffer[4] = Handle.Msb ^ 0x20;
    buffer[5] = 0x13;

    return HCI_Command(Context, HCI_Disconnect, buffer, 6);
}
