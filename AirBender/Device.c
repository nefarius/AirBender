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
#include "device.tmh"
#include <stdlib.h>


NTSTATUS
AirBenderCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    PDEVICE_CONTEXT pDeviceContext;
    WDFDEVICE device;
    NTSTATUS status;
    WDF_DEVICE_PNP_CAPABILITIES pnpCapabilities;

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
    pnpPowerCallbacks.EvtDevicePrepareHardware = AirBenderEvtDevicePrepareHardware;
    pnpPowerCallbacks.EvtDeviceD0Entry = AirBenderEvtDeviceD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = AirBenderEvtDeviceD0Exit;
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoBuffered);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status))
    {
        status = AirBenderChildQueuesInitialize(device);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        pDeviceContext = DeviceGetContext(device);

        BTH_DEVICE_LIST_INIT(&pDeviceContext->ClientDeviceList);

        InitHidInitReports(pDeviceContext);

        WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCapabilities);
        pnpCapabilities.Removable = WdfTrue;
        pnpCapabilities.SurpriseRemovalOK = WdfTrue;
        WdfDeviceSetPnpCapabilities(device, &pnpCapabilities);

        //
        // Create a device interface so that applications can find and talk
        // to us.
        //
        status = WdfDeviceCreateDeviceInterface(
            device,
            &GUID_DEVINTERFACE_AIRBENDER,
            NULL // ReferenceString
        );
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
                "WdfDeviceCreateDeviceInterface failed with status %!STATUS!", status);
            return status;
        }


        //
        // Initialize the I/O Package and any Queues
        //
        status = AirBenderQueueInitialize(device);
        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
                "AirBenderQueueInitialize failed with status %!STATUS!", status);
            return status;
        }
    }

    return status;
}

NTSTATUS
AirBenderEvtDevicePrepareHardware(
    _In_ WDFDEVICE Device,
    _In_ WDFCMRESLIST ResourceList,
    _In_ WDFCMRESLIST ResourceListTranslated
)
/*++

Routine Description:

    In this callback, the driver does whatever is necessary to make the
    hardware ready to use.  In the case of a USB device, this involves
    reading and selecting descriptors.

Arguments:

    Device - handle to a device

Return Value:

    NT status value

--*/
{
    NTSTATUS status;
    PDEVICE_CONTEXT pDeviceContext;
    WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
    WDFUSBPIPE                          pipe;
    WDF_USB_PIPE_INFORMATION            pipeInfo;
    UCHAR                               index;
    UCHAR                               numberConfiguredPipes;

    UNREFERENCED_PARAMETER(ResourceList);
    UNREFERENCED_PARAMETER(ResourceListTranslated);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    status = STATUS_SUCCESS;
    pDeviceContext = DeviceGetContext(Device);

    //
    // Create a USB device handle so that we can communicate with the
    // underlying USB stack. The WDFUSBDEVICE handle is used to query,
    // configure, and manage all aspects of the USB device.
    // These aspects include device properties, bus properties,
    // and I/O creation and synchronization. We only create the device the first time
    // PrepareHardware is called. If the device is restarted by pnp manager
    // for resource rebalance, we will use the same device handle but then select
    // the interfaces again because the USB stack could reconfigure the device on
    // restart.
    //
    if (pDeviceContext->UsbDevice == NULL) {

        status = WdfUsbTargetDeviceCreate(Device,
            WDF_NO_OBJECT_ATTRIBUTES,
            &pDeviceContext->UsbDevice
        );

        if (!NT_SUCCESS(status)) {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
                "WdfUsbTargetDeviceCreateWithParameters failed with status %!STATUS!", status);
            return status;
        }
    }

    //
    // Select the first configuration of the device, using the first alternate
    // setting of each interface
    //
    WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_MULTIPLE_INTERFACES(&configParams,
        0,
        NULL
    );
    status = WdfUsbTargetDeviceSelectConfig(pDeviceContext->UsbDevice,
        WDF_NO_OBJECT_ATTRIBUTES,
        &configParams
    );

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
            "WdfUsbTargetDeviceSelectConfig failed with status %!STATUS!", status);
        return status;
    }

    pDeviceContext->UsbInterface =
        WdfUsbTargetDeviceGetInterface(pDeviceContext->UsbDevice, 0);

    if (NULL == pDeviceContext->UsbInterface) {
        status = STATUS_UNSUCCESSFUL;
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
            "WdfUsbTargetDeviceGetInterface 0 failed with status %!STATUS!",
            status);
        return status;
    }

    numberConfiguredPipes = WdfUsbInterfaceGetNumConfiguredPipes(pDeviceContext->UsbInterface);

    //
    // Get pipe handles
    //
    for (index = 0; index < numberConfiguredPipes; index++) {

        WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);

        pipe = WdfUsbInterfaceGetConfiguredPipe(
            pDeviceContext->UsbInterface,
            index, //PipeIndex,
            &pipeInfo
        );
        //
        // Tell the framework that it's okay to read less than
        // MaximumPacketSize
        //
        WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(pipe);

        if (WdfUsbPipeTypeInterrupt == pipeInfo.PipeType) {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE,
                "Interrupt Pipe is 0x%p\n", pipe);
            pDeviceContext->InterruptPipe = pipe;
        }

        if (WdfUsbPipeTypeBulk == pipeInfo.PipeType &&
            WdfUsbTargetPipeIsInEndpoint(pipe)) {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE,
                "BulkInput Pipe is 0x%p\n", pipe);
            pDeviceContext->BulkReadPipe = pipe;
        }

        if (WdfUsbPipeTypeBulk == pipeInfo.PipeType &&
            WdfUsbTargetPipeIsOutEndpoint(pipe)) {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE,
                "BulkOutput Pipe is 0x%p\n", pipe);
            pDeviceContext->BulkWritePipe = pipe;
        }

    }

    //
    // If we didn't find all the 3 pipes, fail the start.
    //
    if (!(pDeviceContext->BulkWritePipe
        && pDeviceContext->BulkReadPipe && pDeviceContext->InterruptPipe)) {
        status = STATUS_INVALID_DEVICE_STATE;
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
            "Device is not configured properly %!STATUS!\n",
            status);

        return status;
    }

    // TODO: check return values
    AirBenderConfigContReaderForInterruptEndPoint(Device);
    AirBenderConfigContReaderForBulkReadEndPoint(pDeviceContext);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

_Use_decl_annotations_
NTSTATUS
AirBenderEvtDeviceD0Entry(
    WDFDEVICE  Device,
    WDF_POWER_DEVICE_STATE  PreviousState
)
{
    PDEVICE_CONTEXT         pDeviceContext;
    NTSTATUS                status;
    BOOLEAN                 isTargetStarted;

    pDeviceContext = DeviceGetContext(Device);
    isTargetStarted = FALSE;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    UNREFERENCED_PARAMETER(PreviousState);

    //
    // Since continuous reader is configured for this interrupt-pipe, we must explicitly start
    // the I/O target to get the framework to post read requests.
    //
    status = WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(pDeviceContext->InterruptPipe));
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DEVICE,
            "Failed to start interrupt pipe %!STATUS!", status);
        goto End;
    }

    status = WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(pDeviceContext->BulkReadPipe));
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DEVICE,
            "Failed to start bulk read pipe %!STATUS!\n", status);
        goto End;
    }

    status = WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(pDeviceContext->BulkWritePipe));
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR,
            TRACE_DEVICE,
            "Failed to start bulk write pipe %!STATUS!\n", status);
        goto End;
    }

    isTargetStarted = TRUE;

End:

    if (!NT_SUCCESS(status)) {
        //
        // Failure in D0Entry will lead to device being removed. So let us stop the continuous
        // reader in preparation for the ensuing remove.
        //
        if (isTargetStarted) {
            WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(pDeviceContext->InterruptPipe), WdfIoTargetCancelSentIo);
            WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(pDeviceContext->BulkReadPipe), WdfIoTargetCancelSentIo);
            WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(pDeviceContext->BulkWritePipe), WdfIoTargetCancelSentIo);
        }
    }

    HCI_Command_Reset(pDeviceContext);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

_Use_decl_annotations_
NTSTATUS
AirBenderEvtDeviceD0Exit(
    WDFDEVICE  Device,
    WDF_POWER_DEVICE_STATE  TargetState
)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(TargetState);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}

VOID InitHidInitReports(IN PDEVICE_CONTEXT Context)
{
    InitByteArray(&Context->HidInitReports);

    APPEND_BYTE_ARRAY(Context->HidInitReports, P99_PROTECT({
        0x02, 0x00, 0x0F, 0x00, 0x08, 0x35, 0x03, 0x19,
        0x12, 0x00, 0x00, 0x03, 0x00
        }));

    APPEND_BYTE_ARRAY(Context->HidInitReports, P99_PROTECT({
        0x04, 0x00, 0x10, 0x00, 0x0F, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x10, 0x35, 0x06, 0x09, 0x02, 0x01,
        0x09, 0x02, 0x02, 0x00
        }));

    APPEND_BYTE_ARRAY(Context->HidInitReports, P99_PROTECT({
        0x06, 0x00, 0x11, 0x00, 0x0D, 0x35, 0x03, 0x19,
        0x11, 0x24, 0x01, 0x90, 0x35, 0x03, 0x09, 0x02,
        0x06, 0x00
        }));

    APPEND_BYTE_ARRAY(Context->HidInitReports, P99_PROTECT({
        0x06, 0x00, 0x12, 0x00, 0x0F, 0x35, 0x03, 0x19,
        0x11, 0x24, 0x01, 0x90, 0x35, 0x03, 0x09, 0x02,
        0x06, 0x02, 0x00, 0x7F
        }));

    APPEND_BYTE_ARRAY(Context->HidInitReports, P99_PROTECT({
        0x06, 0x00, 0x13, 0x00, 0x0F, 0x35, 0x03, 0x19,
        0x11, 0x24, 0x01, 0x90, 0x35, 0x03, 0x09, 0x02,
        0x06, 0x02, 0x00, 0x59
        }));

    APPEND_BYTE_ARRAY(Context->HidInitReports, P99_PROTECT({
        0x06, 0x00, 0x14, 0x00, 0x0F, 0x35, 0x03, 0x19,
        0x11, 0x24, 0x01, 0x80, 0x35, 0x03, 0x09, 0x02,
        0x06, 0x02, 0x00, 0x33
        }));

    APPEND_BYTE_ARRAY(Context->HidInitReports, P99_PROTECT({
        0x06, 0x00, 0x15, 0x00, 0x0F, 0x35, 0x03, 0x19,
        0x11, 0x24, 0x01, 0x90, 0x35, 0x03, 0x09, 0x02,
        0x06, 0x02, 0x00, 0x0D
        }));
}

