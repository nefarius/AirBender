#include "Driver.h"

#include "interrupt.tmh"


NTSTATUS 
SendControlRequest(
    _In_ PDEVICE_CONTEXT Context,
    _In_ BYTE Type,
    _In_ BYTE Request,
    _In_ USHORT Value,
    _In_ USHORT Index,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength)
{
    NTSTATUS                        status;
    WDF_USB_CONTROL_SETUP_PACKET    controlSetupPacket;
    WDF_REQUEST_SEND_OPTIONS        sendOptions;
    WDF_MEMORY_DESCRIPTOR           memDesc;
    ULONG                           bytesTransferred;

    WDF_REQUEST_SEND_OPTIONS_INIT(
        &sendOptions,
        WDF_REQUEST_SEND_OPTION_TIMEOUT
    );

    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
        &sendOptions,
        DEFAULT_CONTROL_TRANSFER_TIMEOUT
    );

    switch (Type)
    {
    case BmRequestClass:
        WDF_USB_CONTROL_SETUP_PACKET_INIT_CLASS(&controlSetupPacket,
            BmRequestHostToDevice,
            BmRequestToDevice,
            Request,
            Value,
            Index);
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDesc,
        Buffer,
        BufferLength);

    status = WdfUsbTargetDeviceSendControlTransferSynchronously(
        Context->UsbDevice,
        WDF_NO_HANDLE,
        &sendOptions,
        &controlSetupPacket,
        &memDesc,
        &bytesTransferred);

    if (!NT_SUCCESS(status)) {

        TraceEvents(TRACE_LEVEL_ERROR, TRACE_INTERRUPT,
            "WdfUsbTargetDeviceSendControlTransferSynchronously: Failed - 0x%x (%d)\n", status, bytesTransferred);
    }

    return status;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
AirBenderConfigContReaderForInterruptEndPoint(
    _In_ PDEVICE_CONTEXT DeviceContext
)
/*++
Routine Description:
This routine configures a continuous reader on the
interrupt endpoint. It's called from the PrepareHarware event.
Arguments:
Return Value:
NT status value
--*/
{
    WDF_USB_CONTINUOUS_READER_CONFIG contReaderConfig;
    NTSTATUS status;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "%!FUNC! Entry");

    WDF_USB_CONTINUOUS_READER_CONFIG_INIT(&contReaderConfig,
        AirBenderEvtUsbInterruptPipeReadComplete,
        DeviceContext,    // Context
        sizeof(UCHAR));   // TransferLength

    contReaderConfig.EvtUsbTargetPipeReadersFailed = AirBenderEvtUsbInterruptReadersFailed;

    //
    // Reader requests are not posted to the target automatically.
    // Driver must explictly call WdfIoTargetStart to kick start the
    // reader.  In this sample, it's done in D0Entry.
    // By defaut, framework queues two requests to the target
    // endpoint. Driver can configure up to 10 requests with CONFIG macro.
    //
    status = WdfUsbTargetPipeConfigContinuousReader(DeviceContext->InterruptPipe,
        &contReaderConfig);

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_INTERRUPT,
            "OsrFxConfigContReaderForInterruptEndPoint failed %x\n",
            status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "%!FUNC! Exit");

    return status;
}

VOID
AirBenderEvtUsbInterruptPipeReadComplete(
    WDFUSBPIPE  Pipe,
    WDFMEMORY   Buffer,
    size_t      NumBytesTransferred,
    WDFCONTEXT  Context
)
/*++
Routine Description:
This the completion routine of the continour reader. This can
called concurrently on multiprocessor system if there are
more than one readers configured. So make sure to protect
access to global resources.
Arguments:
Buffer - This buffer is freed when this call returns.
If the driver wants to delay processing of the buffer, it
can take an additional referrence.
Context - Provided in the WDF_USB_CONTINUOUS_READER_CONFIG_INIT macro
Return Value:
NT status value
--*/
{
    WDFDEVICE       device;
    PDEVICE_CONTEXT pDeviceContext = Context;

    UNREFERENCED_PARAMETER(Pipe);
    UNREFERENCED_PARAMETER(Buffer);

    device = WdfObjectContextGetObject(pDeviceContext);

    //
    // Make sure that there is data in the read packet.  Depending on the device
    // specification, it is possible for it to return a 0 length read in
    // certain conditions.
    //

    if (NumBytesTransferred == 0) {
        TraceEvents(TRACE_LEVEL_WARNING, TRACE_INTERRUPT,
            "OsrFxEvtUsbInterruptPipeReadComplete Zero length read "
            "occured on the Interrupt Pipe's Continuous Reader\n"
        );
        return;
    }


    //assert(NumBytesTransferred == sizeof(UCHAR));
        
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT,
        "NumBytesTransferred %x\n",
        (ULONG)NumBytesTransferred);

   // pDeviceContext->CurrentSwitchState = *switchState;

    //
    // Handle any pending Interrupt Message IOCTLs. Note that the OSR USB device
    // will generate an interrupt message when the the device resumes from a low
    // power state. So if the Interrupt Message IOCTL was sent after the device
    // has gone to a low power state, the pending Interrupt Message IOCTL will
    // get completed in the function call below, before the user twiddles the
    // dip switches on the OSR USB device. If this is not the desired behavior
    // for your driver, then you could handle this condition by maintaining a
    // state variable on D0Entry to track interrupt messages caused by power up.
    //
    //OsrUsbIoctlGetInterruptMessage(device, STATUS_SUCCESS);

}

BOOLEAN
AirBenderEvtUsbInterruptReadersFailed(
    _In_ WDFUSBPIPE Pipe,
    _In_ NTSTATUS Status,
    _In_ USBD_STATUS UsbdStatus
)
{
    WDFDEVICE device = WdfIoTargetGetDevice(WdfUsbTargetPipeGetIoTarget(Pipe));
    PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(device);

    UNREFERENCED_PARAMETER(UsbdStatus);
    UNREFERENCED_PARAMETER(Status);
    UNREFERENCED_PARAMETER(pDeviceContext);

    //
    // Clear the current switch state.
    //
    //pDeviceContext->CurrentSwitchState = 0;

    //
    // Service the pending interrupt switch change request
    //
    //OsrUsbIoctlGetInterruptMessage(device, Status);

    return TRUE;
}