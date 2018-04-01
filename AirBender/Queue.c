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

NTSTATUS AirBenderChildQueuesInitialize(WDFDEVICE Device)
{
    NTSTATUS                status;
    WDF_IO_QUEUE_CONFIG     queueConfig;
    WDF_OBJECT_ATTRIBUTES   attributes;
    PDEVICE_CONTEXT         pDeviceContext;

    WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = Device;

    pDeviceContext = DeviceGetContext(Device);

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        &attributes,
        &pDeviceContext->ChildDeviceArrivalQueue
    );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE,
            "WdfIoQueueCreate for ChildDeviceArrivalQueue failed with status %!STATUS!",
            status);
        return status;
    }

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        &attributes,
        &pDeviceContext->ChildDeviceRemovalQueue
    );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE,
            "WdfIoQueueCreate for ChildDeviceRemovalQueue failed with status %!STATUS!",
            status);
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
    NTSTATUS                            status;
    PAIRBENDER_GET_HOST_BD_ADDR         pGetBdAddr;
    size_t                              bufferLength;
    ULONG                               transferred = 0;
    PDEVICE_CONTEXT                     pDeviceContext;
    PBTH_DEVICE                         pBthDevice;
    PAIRBENDER_GET_CLIENT_COUNT         pGetClientCount;
    PAIRBENDER_GET_CLIENT_DETAILS       pGetStateReq;
    PAIRBENDER_GET_DS3_INPUT_REPORT     pGetDs3Input;
    PAIRBENDER_SET_DS3_OUTPUT_REPORT    pSetDs3Output;
    PAIRBENDER_GET_HOST_VERSION         pGetHostVersion;
    PVOID                               buffer;

    // TraceEvents(TRACE_LEVEL_INFORMATION,
    //     TRACE_QUEUE,
    //     "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode 0x%X",
    //     Queue, Request, (int)OutputBufferLength, (int)InputBufferLength, IoControlCode);

    pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(Queue));

    switch (IoControlCode)
    {
#pragma region IOCTL_AIRBENDER_GET_HOST_BD_ADDR

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

#pragma endregion

#pragma region IOCTL_AIRBENDER_HOST_RESET

    case IOCTL_AIRBENDER_HOST_RESET:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_HOST_RESET");

        pDeviceContext->Started = FALSE;
        status = HCI_Command_Reset(pDeviceContext);

        BTH_DEVICE_LIST_FREE(&pDeviceContext->ClientDeviceList);
        BTH_DEVICE_LIST_INIT(&pDeviceContext->ClientDeviceList);

        break;

#pragma endregion

#pragma region IOCTL_AIRBENDER_GET_CLIENT_COUNT

    case IOCTL_AIRBENDER_GET_CLIENT_COUNT:

        // TraceEvents(TRACE_LEVEL_INFORMATION,
        //     TRACE_QUEUE, "IOCTL_AIRBENDER_GET_CLIENT_COUNT");

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

#pragma endregion

#pragma region IOCTL_AIRBENDER_GET_CLIENT_DETAILS

    case IOCTL_AIRBENDER_GET_CLIENT_DETAILS:

        // TraceEvents(TRACE_LEVEL_INFORMATION,
        //     TRACE_QUEUE, "IOCTL_AIRBENDER_GET_CLIENT_DETAILS");

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

#pragma endregion

#pragma region IOCTL_AIRBENDER_GET_DS3_INPUT_REPORT

    case IOCTL_AIRBENDER_GET_DS3_INPUT_REPORT:

        TraceEvents(TRACE_LEVEL_VERBOSE,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_DS3_INPUT_REPORT");

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(AIRBENDER_GET_DS3_INPUT_REPORT),
            (LPVOID)&pGetDs3Input,
            &bufferLength);

        if (NT_SUCCESS(status) && InputBufferLength == sizeof(AIRBENDER_GET_DS3_INPUT_REPORT))
        {
            pBthDevice = BTH_DEVICE_LIST_GET_BY_BD_ADDR(&pDeviceContext->ClientDeviceList, &pGetDs3Input->ClientAddress);

            if (pBthDevice == NULL)
            {
                TraceEvents(TRACE_LEVEL_INFORMATION,
                    TRACE_QUEUE, "Device not found");

                status = STATUS_DEVICE_DOES_NOT_EXIST;
                break;
            }

            status = WdfRequestForwardToIoQueue(Request, pBthDevice->HidInputReportQueue);
            if (!NT_SUCCESS(status))
            {
                TraceEvents(TRACE_LEVEL_ERROR,
                    TRACE_QUEUE, "WdfRequestForwardToIoQueue failed with status %!STATUS!", status);
                break;
            }

            status = STATUS_PENDING;
        }

        break;

#pragma endregion

#pragma region IOCTL_AIRBENDER_SET_DS3_OUTPUT_REPORT

    case IOCTL_AIRBENDER_SET_DS3_OUTPUT_REPORT:

        TraceEvents(TRACE_LEVEL_VERBOSE,
            TRACE_QUEUE, "IOCTL_AIRBENDER_SET_DS3_OUTPUT_REPORT");

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(AIRBENDER_SET_DS3_OUTPUT_REPORT),
            (LPVOID)&pSetDs3Output,
            &bufferLength);

        if (NT_SUCCESS(status) && InputBufferLength == sizeof(AIRBENDER_SET_DS3_OUTPUT_REPORT))
        {
            pBthDevice = BTH_DEVICE_LIST_GET_BY_BD_ADDR(&pDeviceContext->ClientDeviceList, &pSetDs3Output->ClientAddress);

            if (pBthDevice == NULL)
            {
                TraceEvents(TRACE_LEVEL_INFORMATION,
                    TRACE_QUEUE, "Device not found");

                status = STATUS_DEVICE_DOES_NOT_EXIST;
                break;
            }

            buffer = WdfMemoryGetBuffer(pBthDevice->HidOutputReportMemory, &bufferLength);

            RtlCopyMemory(buffer, pSetDs3Output->ReportBuffer, bufferLength);
        }

        break;

#pragma endregion

#pragma region IOCTL_AIRBENDER_PORT_RESET

    case IOCTL_AIRBENDER_HOST_SHUTDOWN:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_HOST_SHUTDOWN");

        pDeviceContext->Started = TRUE; // suppresses boot-up after reset
        status = HCI_Command_Reset(pDeviceContext);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_WARNING,
                TRACE_QUEUE,
                "HCI_Command_Reset failed with status %!STATUS!", status);
        }

        BTH_DEVICE_LIST_FREE(&pDeviceContext->ClientDeviceList);
        BTH_DEVICE_LIST_INIT(&pDeviceContext->ClientDeviceList);

        status = WdfUsbTargetDeviceResetPortSynchronously(pDeviceContext->UsbDevice);
        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE,
                "WdfUsbTargetDeviceResetPortSynchronously failed with status %!STATUS!",
                status);
        }

        break;

#pragma endregion

#pragma region IOCTL_AIRBENDER_GET_CLIENT_ARRIVAL

    case IOCTL_AIRBENDER_GET_CLIENT_ARRIVAL:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_CLIENT_ARRIVAL");

        status = WdfRequestForwardToIoQueue(Request, pDeviceContext->ChildDeviceArrivalQueue);
        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE, "WdfRequestForwardToIoQueue failed with status %!STATUS!", status);
            break;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "Request queued");

        status = STATUS_PENDING;

        break;

#pragma endregion

#pragma region IOCTL_AIRBENDER_GET_CLIENT_REMOVAL

    case IOCTL_AIRBENDER_GET_CLIENT_REMOVAL:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_CLIENT_REMOVAL");

        status = WdfRequestForwardToIoQueue(Request, pDeviceContext->ChildDeviceRemovalQueue);
        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE, "WdfRequestForwardToIoQueue failed with status %!STATUS!", status);
            break;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "Request queued");

        status = STATUS_PENDING;

        break;

#pragma endregion

#pragma region IOCTL_AIRBENDER_GET_HOST_VERSION

    case IOCTL_AIRBENDER_GET_HOST_VERSION:

        TraceEvents(TRACE_LEVEL_INFORMATION,
            TRACE_QUEUE, "IOCTL_AIRBENDER_GET_HOST_VERSION");

        status = WdfRequestRetrieveOutputBuffer(
            Request,
            sizeof(AIRBENDER_GET_HOST_VERSION),
            (LPVOID)&pGetHostVersion,
            &bufferLength);

        if (NT_SUCCESS(status) && OutputBufferLength == sizeof(AIRBENDER_GET_HOST_VERSION))
        {
            transferred = sizeof(AIRBENDER_GET_HOST_VERSION);
            pGetHostVersion->HciVersionMajor = pDeviceContext->HciVersionMajor;
            pGetHostVersion->LmpVersionMajor = pDeviceContext->LmpVersionMajor;
        }
        else
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE, "Buffer size mismatch: %d != %d",
                (ULONG)OutputBufferLength, sizeof(AIRBENDER_GET_HOST_VERSION));
        }

        break;

#pragma endregion

    default:
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_QUEUE, "Unknown IOCTL code");
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (status != STATUS_PENDING)
        WdfRequestCompleteWithInformation(Request, status, transferred);

    // TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! Exit");
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
AirBenderWriteBulkPipeEvtIoDefault(
    WDFQUEUE  Queue,
    WDFREQUEST  Request
)
{
    NTSTATUS                            status;
    PDEVICE_CONTEXT                     pDeviceCtx;
    size_t                              bufferLength;
    PAIRBENDER_SET_DS3_OUTPUT_REPORT    pSetDs3Output;
    PBTH_DEVICE                         pBthDevice;


    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_QUEUE, "%!FUNC! Entry");

    pDeviceCtx = DeviceGetContext(WdfIoQueueGetDevice(Queue));

    status = WdfRequestRetrieveInputBuffer(
        Request,
        sizeof(AIRBENDER_SET_DS3_OUTPUT_REPORT),
        (LPVOID)&pSetDs3Output,
        &bufferLength);

    if (NT_SUCCESS(status) && bufferLength == sizeof(AIRBENDER_SET_DS3_OUTPUT_REPORT))
    {
        L2CAP_CID scid;

        pBthDevice = BTH_DEVICE_LIST_GET_BY_BD_ADDR(&pDeviceCtx->ClientDeviceList, &pSetDs3Output->ClientAddress);

        if (pBthDevice == NULL)
        {
            TraceEvents(TRACE_LEVEL_INFORMATION,
                TRACE_QUEUE, "Device not found");

            status = STATUS_DEVICE_DOES_NOT_EXIST;
            WdfRequestComplete(Request, status);
            return;
        }

        L2CAP_DEVICE_GET_SCID_FOR_TYPE(
            pBthDevice,
            L2CAP_PSM_HID_Command,
            &scid);

        status = HID_Command(
            pDeviceCtx,
            pBthDevice->HCI_ConnectionHandle,
            scid,
            pSetDs3Output->ReportBuffer,
            DS3_HID_OUTPUT_REPORT_SIZE);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR,
                TRACE_QUEUE, "HID_Command failed with status %!STATUS!", status);
        }
    }

    WdfRequestComplete(Request, status);

    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_QUEUE, "%!FUNC! Exit");
}

