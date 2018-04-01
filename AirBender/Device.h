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


#include "Bluetooth.h"

EXTERN_C_START

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    WDFUSBDEVICE UsbDevice;

    WDFUSBINTERFACE UsbInterface;
    
    WDFUSBPIPE InterruptPipe;

    WDFUSBPIPE BulkReadPipe;

    WDFUSBPIPE BulkWritePipe;

    BOOLEAN DisableSSP;

    BOOLEAN Started;

    BD_ADDR BluetoothHostAddress;

    BOOLEAN Initialized;

    BTH_DEVICE_LIST ClientDeviceList;

    BYTE_ARRAY HidInitReports;

    WDFQUEUE ChildDeviceArrivalQueue;

    WDFQUEUE ChildDeviceRemovalQueue;

    BYTE HciVersionMajor;

    BYTE LmpVersionMajor;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called DeviceGetContext
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

//
// Function to initialize the device's queues and callbacks
//
NTSTATUS
AirBenderCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

//
// Function to select the device's USB configuration and get a WDFUSBDEVICE
// handle
//
EVT_WDF_DEVICE_PREPARE_HARDWARE AirBenderEvtDevicePrepareHardware;
EVT_WDF_DEVICE_D0_ENTRY AirBenderEvtDeviceD0Entry;
EVT_WDF_DEVICE_D0_EXIT AirBenderEvtDeviceD0Exit;

VOID
InitHidInitReports(
    IN PDEVICE_CONTEXT Context);

EXTERN_C_END
