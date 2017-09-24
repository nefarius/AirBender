/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "queue.tmh"
#include "L2CAP.h"

NTSTATUS
AirBenderQueueInitialize(
    _In_ WDFDEVICE Device
)
/*++

Routine Description:


     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG    queueConfig;

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &queueConfig,
        WdfIoQueueDispatchParallel
    );

    queueConfig.EvtIoDeviceControl = AirBenderEvtIoDeviceControl;
    queueConfig.EvtIoStop = AirBenderEvtIoStop;
    queueConfig.EvtIoRead = AirBenderEvtIoRead;

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &queue
    );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
        return status;
    }

    return status;
}

VOID
AirBenderEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
)
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{
    NTSTATUS                            status = STATUS_SUCCESS;
    PAIRBENDER_GET_HOST_BD_ADDR         pGetBdAddr;
    SIZE_T                              bufferLength;
    ULONG                               transferred = 0;
    PDEVICE_CONTEXT                     pDeviceContext;
    PBTH_DEVICE                         pBthDevice;
    PAIRBENDER_GET_CLIENT_COUNT         pGetClientCount;
    PAIRBENDER_GET_CLIENT_DETAILS       pGetStateReq;
    PAIRBENDER_GET_DS3_INPUT_REPORT     pGetDs3Input;
    PAIRBENDER_SET_DS3_OUTPUT_REPORT    pSetDs3Output;

    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_QUEUE,
        "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode 0x%X",
        Queue, Request, (int)OutputBufferLength, (int)InputBufferLength, IoControlCode);

    pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(Queue));

    switch (IoControlCode)
    {
    case IOCTL_AIRBENDER_GET_HOST_BD_ADDR:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_HOST_BD_ADDR");

        status = WdfRequestRetrieveOutputBuffer(
            Request,
            sizeof(AIRBENDER_GET_HOST_BD_ADDR),
            (LPVOID)&pGetBdAddr,
            &bufferLength);

        if (NT_SUCCESS(status) && OutputBufferLength == sizeof(AIRBENDER_GET_HOST_BD_ADDR))
        {
            transferred = sizeof(BD_ADDR);
            RtlCopyMemory(&pGetBdAddr->Host, &pDeviceContext->BluetoothHostAddress, transferred);
        }
        else
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE, "Buffer size mismatch: %d != %d",
                (ULONG)OutputBufferLength, sizeof(AIRBENDER_GET_HOST_BD_ADDR));
        }

        break;

    case IOCTL_AIRBENDER_HOST_RESET:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_HOST_RESET");

        pDeviceContext->Started = FALSE;
        status = HCI_Command_Reset(pDeviceContext);

        BTH_DEVICE_LIST_FREE(&pDeviceContext->ClientDeviceList);
        BTH_DEVICE_LIST_INIT(&pDeviceContext->ClientDeviceList);

        break;

    case IOCTL_AIRBENDER_GET_CLIENT_COUNT:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_CLIENT_COUNT");

        status = WdfRequestRetrieveOutputBuffer(
            Request,
            sizeof(AIRBENDER_GET_CLIENT_COUNT),
            (LPVOID)&pGetClientCount,
            &bufferLength);

        if (NT_SUCCESS(status) && OutputBufferLength == sizeof(AIRBENDER_GET_CLIENT_COUNT))
        {
            transferred = sizeof(AIRBENDER_GET_CLIENT_COUNT);
            pGetClientCount->Count = BTH_DEVICE_LIST_GET_COUNT(&pDeviceContext->ClientDeviceList);
        }
        else
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE, "Buffer size mismatch: %d != %d",
                (ULONG)OutputBufferLength, sizeof(AIRBENDER_GET_CLIENT_COUNT));
        }

        break;

    case IOCTL_AIRBENDER_GET_CLIENT_DETAILS:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_CLIENT_DETAILS");

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(AIRBENDER_GET_CLIENT_DETAILS),
            (LPVOID)&pGetStateReq,
            &bufferLength);

        if (NT_SUCCESS(status) && InputBufferLength == sizeof(AIRBENDER_GET_CLIENT_DETAILS))
        {
            AIRBENDER_GET_CLIENT_DETAILS req = *pGetStateReq;

            TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_QUEUE, "Requesting state for device #%d", pGetStateReq->ClientIndex);

            pBthDevice = BTH_DEVICE_LIST_GET_BY_INDEX(&pDeviceContext->ClientDeviceList, pGetStateReq->ClientIndex);

            if (pBthDevice == NULL)
            {
                TraceEvents(TRACE_LEVEL_INFORMATION,
                    TRACE_QUEUE, "Device not found");

                status = STATUS_DEVICE_DOES_NOT_EXIST;
                break;
            }

            status = WdfRequestRetrieveOutputBuffer(
                Request,
                sizeof(AIRBENDER_GET_CLIENT_DETAILS),
                (LPVOID)&pGetStateReq,
                &bufferLength);

            if (NT_SUCCESS(status) && OutputBufferLength == sizeof(AIRBENDER_GET_CLIENT_DETAILS))
            {
                pGetStateReq->ClientIndex = req.ClientIndex;
                pGetStateReq->DeviceType = pBthDevice->DeviceType;

                transferred = sizeof(AIRBENDER_GET_CLIENT_DETAILS);

                pGetStateReq->ClientAddress = pBthDevice->ClientAddress;
            }

            TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_QUEUE, "Done");
        }

        break;

    case IOCTL_AIRBENDER_GET_DS3_INPUT_REPORT:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_DS3_INPUT_REPORT");

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(AIRBENDER_GET_DS3_INPUT_REPORT),
            (LPVOID)&pGetDs3Input,
            &bufferLength);

        if (NT_SUCCESS(status) && InputBufferLength == sizeof(AIRBENDER_GET_DS3_INPUT_REPORT))
        {
            AIRBENDER_GET_DS3_INPUT_REPORT report = *pGetDs3Input;

            pBthDevice = BTH_DEVICE_LIST_GET_BY_BD_ADDR(&pDeviceContext->ClientDeviceList, &report.ClientAddress);

            if (pBthDevice == NULL)
            {
                TraceEvents(TRACE_LEVEL_INFORMATION,
                    TRACE_QUEUE, "Device not found");

                status = STATUS_DEVICE_DOES_NOT_EXIST;
                break;
            }

            status = WdfRequestRetrieveOutputBuffer(
                Request,
                sizeof(AIRBENDER_GET_DS3_INPUT_REPORT),
                (LPVOID)&pGetDs3Input,
                &bufferLength);

            if (NT_SUCCESS(status) && OutputBufferLength == sizeof(AIRBENDER_GET_DS3_INPUT_REPORT))
            {
                pGetDs3Input->ClientAddress = report.ClientAddress;

                transferred = sizeof(AIRBENDER_GET_DS3_INPUT_REPORT);

                RtlCopyMemory(&pGetDs3Input->ReportBuffer,
                    pBthDevice->HidInputReport.Data,
                    pBthDevice->HidInputReport.Length);
            }
        }

        break;

    case IOCTL_AIRBENDER_SET_DS3_OUTPUT_REPORT:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_CLIENT_DETAILS");

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(AIRBENDER_SET_DS3_OUTPUT_REPORT),
            (LPVOID)&pSetDs3Output,
            &bufferLength);

        if (NT_SUCCESS(status) && InputBufferLength == sizeof(AIRBENDER_SET_DS3_OUTPUT_REPORT))
        {
            L2CAP_CID scid;

            pBthDevice = BTH_DEVICE_LIST_GET_BY_BD_ADDR(&pDeviceContext->ClientDeviceList, &pSetDs3Output->ClientAddress);

            if (pBthDevice == NULL)
            {
                TraceEvents(TRACE_LEVEL_INFORMATION,
                    TRACE_QUEUE, "Device not found");

                status = STATUS_DEVICE_DOES_NOT_EXIST;
                break;
            }

            L2CAP_DEVICE_GET_SCID_FOR_TYPE(
                pBthDevice,
                L2CAP_PSM_HID_Command,
                &scid);

            status = HID_Command(
                pDeviceContext,
                pBthDevice->HCI_ConnectionHandle,
                scid,
                pSetDs3Output->ReportBuffer,
                DS3_HID_OUTPUT_REPORT_SIZE);

            if (!NT_SUCCESS(status))
            {
                TraceEvents(TRACE_LEVEL_ERROR,
                    TRACE_QUEUE, "HID_Command failed");
            }
        }

        break;
    default:
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_QUEUE, "Unknown IOCTL code");
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    WdfRequestCompleteWithInformation(Request, status, transferred);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! Exit");
}

VOID
AirBenderEvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
/*++

Routine Description:

    This event is invoked for a power-managed queue before the device leaves the working state (D0).

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
                  that identify the reason that the callback function is being called
                  and whether the request is cancelable.

Return Value:

    VOID

--*/
{
    TraceEvents(TRACE_LEVEL_INFORMATION,
        TRACE_QUEUE,
        "%!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d",
        Queue, Request, ActionFlags);

    //
    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
    // further processing of the I/O request.
    //
    // Typically, the driver uses the following rules:
    //
    // - If the driver owns the I/O request, it either postpones further processing
    //   of the request and calls WdfRequestStopAcknowledge, or it calls WdfRequestComplete
    //   with a completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
    //  
    //   The driver must call WdfRequestComplete only once, to either complete or cancel
    //   the request. To ensure that another thread does not call WdfRequestComplete
    //   for the same request, the EvtIoStop callback must synchronize with the driver's
    //   other event callback functions, for instance by using interlocked operations.
    //
    // - If the driver has forwarded the I/O request to an I/O target, it either calls
    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
    //   further processing of the request and calls WdfRequestStopAcknowledge.
    //
    // A driver might choose to take no action in EvtIoStop for requests that are
    // guaranteed to complete in a small amount of time. For example, the driver might
    // take no action for requests that are completed in one of the driver’s request handlers.
    //

    return;
}

_Use_decl_annotations_
VOID
AirBenderEvtIoRead(
    WDFQUEUE  Queue,
    WDFREQUEST  Request,
    size_t  Length
)
{
    NTSTATUS status;
    WDFMEMORY mem;
    PVOID buffer;
    size_t length;
    PDEVICE_CONTEXT pDeviceContext;

    UNREFERENCED_PARAMETER(Length);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! Entry");

    //
    // TODO: PoC hack, re-implement!


    pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(Queue));

    status = WdfRequestRetrieveOutputMemory(Request, &mem);
    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE,
            "WdfRequestRetrieveOutputMemory failed with status 0x%X", status);
    }

    buffer = WdfMemoryGetBuffer(mem, &length);

    RtlCopyMemory(buffer, pDeviceContext->HidInputReport, 96);

    WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, length);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! Exit");
}
