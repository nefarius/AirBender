/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that app can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_AirBender,
    0xbfb233d2,0x084f,0x4a81,0xaf,0x85,0xe5,0xdd,0xce,0x3a,0xf6,0x57);
// {bfb233d2-084f-4a81-af85-e5ddce3af657}
