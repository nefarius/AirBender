/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    User-mode Driver Framework 2

--*/

#include <windows.h>
#include <wdf.h>
#include <usb.h>
#include <wdfusb.h>
#include <initguid.h>

#include "ByteArray.h"
#include "device.h"
#include "queue.h"
#include "Bluetooth.h"
#include "Interrupt.h"
#include "Bulkrwr.h"
#include "HCI.h"
#include "trace.h"

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD AirBenderEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP AirBenderEvtDriverContextCleanup;

EXTERN_C_END
