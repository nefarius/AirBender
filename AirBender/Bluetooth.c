#include "Driver.h"
#include "Bluetooth.tmh"
#include "L2CAP.h"

_Use_decl_annotations_
VOID
AirBenderBulkWriteEvtTimerFunc(
    WDFTIMER  Timer
)
{
    NTSTATUS                status;
    PBTH_DEVICE_CONTEXT     pBluetoothCtx;
    PBTH_DEVICE             pBthDevice;
    PDEVICE_CONTEXT         pDeviceContext;
    L2CAP_CID               scid;
    PVOID                   buffer;
    size_t                  bufferLength;

    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_BLUETOOTH, "%!FUNC! Entry");

    pBluetoothCtx = BluetoothDeviceGetContext(Timer);
    pDeviceContext = DeviceGetContext(pBluetoothCtx->HostDevice);
    pBthDevice = pBluetoothCtx->Device;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BLUETOOTH,
        "%!FUNC! ClientAddress: %02X:%02X:%02X:%02X:%02X:%02X",
        pBluetoothCtx->Device->ClientAddress.Address[0],
        pBluetoothCtx->Device->ClientAddress.Address[1],
        pBluetoothCtx->Device->ClientAddress.Address[2],
        pBluetoothCtx->Device->ClientAddress.Address[3],
        pBluetoothCtx->Device->ClientAddress.Address[4],
        pBluetoothCtx->Device->ClientAddress.Address[5]);

    buffer = WdfMemoryGetBuffer(pBthDevice->HidOutputReportMemory, &bufferLength);

    L2CAP_DEVICE_GET_SCID_FOR_TYPE(
        pBthDevice,
        L2CAP_PSM_HID_Command,
        &scid);

    status = HID_Command(
        pDeviceContext,
        pBthDevice->HCI_ConnectionHandle,
        scid,
        buffer,
        (ULONG)bufferLength);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_BLUETOOTH, 
            "HID_Command failed with status %!STATUS!", status);
    }

    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_BLUETOOTH, "%!FUNC! Exit");
}
