#include "Driver.h"

#pragma optimize( "", off )
__declspec(noinline)
VOID
BTH_ADD_DEVICE(
    PBTH_DEVICE DeviceList,
    BD_ADDR Address)
{
    PBTH_DEVICE device = (PBTH_DEVICE)malloc(sizeof(BTH_DEVICE));
    RtlZeroMemory(device, sizeof(BTH_DEVICE));

    device->ClientAddress = Address;

    HASH_ADD(hh, DeviceList, ClientAddress, sizeof(BD_ADDR), device);

}
#pragma optimize( "", on )  
