#pragma once


#define BD_LINK_LENGTH  0x10
#include <stdlib.h>

static const BYTE BD_LINK[BD_LINK_LENGTH] =
{
    0x56, 0xE8, 0x81, 0x38, 0x08, 0x06, 0x51, 0x41,
    0xC0, 0x7F, 0x12, 0xAA, 0xD9, 0x66, 0x3C, 0xCE
};


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

    RtlCopyMemory(&node->ClientAddress, Address, sizeof(BD_ADDR));

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
    BD_ADDR Address
)
{
    PBTH_DEVICE node = List->head;

    while (node != NULL)
    {
        if (node->ClientAddress.Address == Address.Address)
        {
            return node;
        }

        node = node->next;
    }

    return NULL;
}
