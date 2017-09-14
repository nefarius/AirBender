// ABD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>

#include "../uthash/src/uthash.h"




typedef struct _BD_ADDR
{
    BYTE Address[6];

} BD_ADDR;

typedef struct _BTH_HANDLE
{
    BYTE Lsb;
    BYTE Msb;

} BTH_HANDLE;

typedef struct _BTH_DEVICE
{
    BD_ADDR ClientAddress;

    BTH_HANDLE ConnectionHandle;

    UT_hash_handle hh;

    struct BTH_DEVICE *next;

} BTH_DEVICE2, *PBTH_DEVICE;

VOID FORCEINLINE
BTH_ADD_DEVICE(
    PBTH_DEVICE DeviceList,
    BD_ADDR Address)
{
    PBTH_DEVICE device = (PBTH_DEVICE)malloc(sizeof(BTH_DEVICE2));
    RtlZeroMemory(device, sizeof(BTH_DEVICE2));

    device->ClientAddress = Address;

    HASH_ADD(hh, DeviceList, ClientAddress, sizeof(BD_ADDR), device);
}

VOID FORCEINLINE
BTH_REMOVE_DEVICE(
    PBTH_DEVICE DeviceList,
    PBTH_DEVICE Device)
{
    HASH_DEL(DeviceList, Device);
    free(Device);
}

ULONG FORCEINLINE
BTH_GET_DEVICE_COUNT(
    PBTH_DEVICE DeviceList)
{
    return HASH_COUNT(DeviceList);
}


int main()
{
    auto device = L"\\\\?\\usb#vid_0a12&pid_0001#6&22bce1d6&0&3#{a5dcbf10-6530-11d2-901f-00c04fb951ed}";

    PBTH_DEVICE dev = NULL;
    BD_ADDR addr = {0};
    BTH_ADD_DEVICE(dev, addr);

    return 0;
}

