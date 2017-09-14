#pragma once


#define BD_LINK_LENGTH  0x10
#include <stdlib.h>

static const BYTE BD_LINK[BD_LINK_LENGTH] =
{
    0x56, 0xE8, 0x81, 0x38, 0x08, 0x06, 0x51, 0x41,
    0xC0, 0x7F, 0x12, 0xAA, 0xD9, 0x66, 0x3C, 0xCE
};

LPCSTR FORCEINLINE HCI_ERROR_DETAIL(
    BYTE Error
)
{
    switch (Error)
    {
    case 0x00: return "Success";
    case 0x01: return "Unknown HCI Command";
    case 0x02: return "Unknown Connection Identifier";
    case 0x03: return "Hardware Failure";
    case 0x04: return "Page Timeout";
    case 0x05: return "Authentication Failure";
    case 0x06: return "PIN or Key Missing";
    case 0x07: return "Memory Capacity Exceeded";
    case 0x08: return "Connection Timeout";
    case 0x09: return "Connection Limit Exceeded";
    case 0x0A: return "Synchronous Connection Limit To A Device Exceeded";
    case 0x0B: return "ACL Connection Already Exists";
    case 0x0C: return "Command Disallowed";
    case 0x0D: return "Connection Rejected due to Limited Resources";
    case 0x0E: return "Connection Rejected Due To Security Reasons";
    case 0x0F: return "Connection Rejected due to Unacceptable BD_ADDR";
    case 0x10: return "Connection Accept Timeout Exceeded";
    case 0x11: return "Unsupported Feature or Parameter Value";
    case 0x12: return "Invalid HCI Command Parameters";
    case 0x13: return "Remote User Terminated Connection";
    case 0x14: return "Remote Device Terminated Connection due to Low Resources";
    case 0x15: return "Remote Device Terminated Connection due to Power Off";
    case 0x16: return "Connection Terminated By Local Host";
    case 0x17: return "Repeated Attempts";
    case 0x18: return "Pairing Not Allowed";
    case 0x19: return "Unknown LMP PDU";
    case 0x1A: return "Unsupported Remote Feature / Unsupported LMP Feature";
    case 0x1B: return "SCO Offset Rejected";
    case 0x1C: return "SCO Interval Rejected";
    case 0x1D: return "SCO Air Mode Rejected";
    case 0x1E: return "Invalid LMP Parameters";
    case 0x1F: return "Unspecified Error";
    case 0x20: return "Unsupported LMP Parameter Value";
    case 0x21: return "Role Change Not Allowed";
    case 0x22: return "LMP Response Timeout / LL Response Timeout";
    case 0x23: return "LMP Error Transaction Collision";
    case 0x24: return "LMP PDU Not Allowed";
    case 0x25: return "Encryption Mode Not Acceptable";
    case 0x26: return "Link Key cannot be Changed";
    case 0x27: return "Requested QoS Not Supported";
    case 0x28: return "Instant Passed";
    case 0x29: return "Pairing With Unit Key Not Supported";
    case 0x2A: return "Different Transaction Collision";
    case 0x2B: return "Reserved";
    case 0x2C: return "QoS Unacceptable Parameter";
    case 0x2D: return "QoS Rejected";
    case 0x2E: return "Channel Classification Not Supported";
    case 0x2F: return "Insufficient Security";
    case 0x30: return "Parameter Out Of Mandatory Range";
    case 0x31: return "Reserved";
    case 0x32: return "Role Switch Pending";
    case 0x33: return "Reserved";
    case 0x34: return "Reserved Slot Violation";
    case 0x35: return "Role Switch Failed";
    case 0x36: return "Extended Inquiry Response Too Large";
    case 0x37: return "Secure Simple Pairing Not Supported By Host.";
    case 0x38: return "Host Busy - Pairing";
    case 0x39: return "Connection Rejected due to No Suitable Channel Found";
    case 0x3A: return "Controller Busy";
    case 0x3B: return "Unacceptable Connection Interval";
    case 0x3C: return "Directed Advertising Timeout";
    case 0x3D: return "Connection Terminated due to MIC Failure";
    case 0x3E: return "Connection Failed to be Established";
    case 0x3F: return "MAC Connection Failed";
    default: return NULL;
    }
}


typedef struct _BD_ADDR
{
    BYTE Address[6];

} BD_ADDR, *PBD_ADDR;

typedef struct _BTH_HANDLE
{
    BYTE Lsb;
    BYTE Msb;

} BTH_HANDLE, *PBTH_HANDLE;

typedef struct _BTH_DEVICE
{
    BD_ADDR ClientAddress;

    BTH_HANDLE ConnectionHandle;

    struct _BTH_DEVICE *next;

} BTH_DEVICE, *PBTH_DEVICE;

typedef struct _BTH_DEVICE_LIST
{
    ULONG logicalLength;

    PBTH_DEVICE head;

    PBTH_DEVICE tail;

} BTH_DEVICE_LIST, *PBTH_DEVICE_LIST;

VOID FORCEINLINE BTH_DEVICE_LIST_INIT(
    PBTH_DEVICE_LIST List
)
{
    List->logicalLength = 0;
    List->head = List->tail = NULL;
}

VOID FORCEINLINE BTH_DEVICE_LIST_ADD(
    PBTH_DEVICE_LIST List,
    PBD_ADDR Address
)
{
    PBTH_DEVICE node = malloc(sizeof(BTH_DEVICE));
    RtlZeroMemory(node, sizeof(BTH_DEVICE));

    node->ClientAddress = *Address;

    if (List->logicalLength == 0) {
        List->head = List->tail = node;
    }
    else {
        List->tail->next = node;
        List->tail = node;
    }

    List->logicalLength++;
}

ULONG FORCEINLINE BTH_DEVICE_LIST_GET_COUNT(
    PBTH_DEVICE_LIST List
)
{
    return List->logicalLength;
}

PBTH_DEVICE FORCEINLINE BTH_DEVICE_LIST_GET_BY_BD_ADDR(
    PBTH_DEVICE_LIST List,
    PBD_ADDR Address
)
{
    PBTH_DEVICE node = List->head;

    while (node != NULL)
    {
        if (node->ClientAddress.Address == Address->Address)
        {
            return node;
        }

        node = node->next;
    }

    return NULL;
}
