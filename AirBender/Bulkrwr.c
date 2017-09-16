#include "Driver.h"
#include "L2CAP.h"

#include "bulkrwr.tmh"

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
AirBenderConfigContReaderForBulkReadEndPoint(
    _In_ PDEVICE_CONTEXT DeviceContext
)
{
    WDF_USB_CONTINUOUS_READER_CONFIG contReaderConfig;
    NTSTATUS status;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "%!FUNC! Entry");

    WDF_USB_CONTINUOUS_READER_CONFIG_INIT(&contReaderConfig,
        AirBenderEvtUsbBulkReadPipeReadComplete,
        DeviceContext,    // Context
        BULK_IN_BUFFER_LENGTH);   // TransferLength

    contReaderConfig.EvtUsbTargetPipeReadersFailed = AirBenderEvtUsbBulkReadReadersFailed;

    //
    // Reader requests are not posted to the target automatically.
    // Driver must explictly call WdfIoTargetStart to kick start the
    // reader.  In this sample, it's done in D0Entry.
    // By defaut, framework queues two requests to the target
    // endpoint. Driver can configure up to 10 requests with CONFIG macro.
    //
    status = WdfUsbTargetPipeConfigContinuousReader(DeviceContext->BulkReadPipe,
        &contReaderConfig);

    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
            "OsrFxConfigContReaderForInterruptEndPoint failed %x\n",
            status);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "%!FUNC! Exit");

    return status;
}

NTSTATUS WriteBulkPipe(
    PDEVICE_CONTEXT Context,
    PVOID Buffer,
    ULONG BufferLength,
    PULONG BytesWritten)
{
    NTSTATUS                        status;
    WDF_MEMORY_DESCRIPTOR           memDesc;

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
        &memDesc,
        Buffer,
        BufferLength
    );

    status = WdfUsbTargetPipeWriteSynchronously(
        Context->BulkWritePipe,
        NULL,
        NULL,
        &memDesc,
        BytesWritten
    );

    return status;
}

VOID
AirBenderEvtUsbBulkReadPipeReadComplete(
    WDFUSBPIPE  Pipe,
    WDFMEMORY   Buffer,
    size_t      NumBytesTransferred,
    WDFCONTEXT  Context
)
{
    NTSTATUS                        status;
    PDEVICE_CONTEXT                 pDeviceContext = Context;
    PUCHAR                          buffer;
    BTH_HANDLE                      clientHandle;
    PBTH_DEVICE                     pClientDevice;
    L2CAP_CID                       dcid = { 0 };
    L2CAP_CID                       scid = { 0 };
    L2CAP_SIGNALLING_COMMAND_CODE   code;

    UNREFERENCED_PARAMETER(status);
    UNREFERENCED_PARAMETER(Pipe);
    UNREFERENCED_PARAMETER(pDeviceContext);

    //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "%!FUNC! Entry");

    if (NumBytesTransferred == 0) {
        TraceEvents(TRACE_LEVEL_WARNING, TRACE_BULKRWR,
            "!FUNC! Zero length read "
            "occurred on the Interrupt Pipe's Continuous Reader\n"
        );
        return;
    }

    buffer = WdfMemoryGetBuffer(Buffer, NULL);

    BTH_HANDLE_FROM_BUFFER(clientHandle, buffer);

    pClientDevice = BTH_DEVICE_LIST_GET_BY_HANDLE(&pDeviceContext->ClientDeviceList, &clientHandle);

    //if (pClientDevice != NULL)
    //    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "Device found");

    if (L2CAP_IS_CONTROL_CHANNEL(buffer))
    {
        //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_IS_CONTROL_CHANNEL");

        if (L2CAP_IS_SIGNALLING_COMMAND_CODE(buffer))
        {
            code = L2CAP_GET_SIGNALLING_COMMAND_CODE(buffer);

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_IS_SIGNALLING_COMMAND_CODE: 0x%02X", (BYTE)code);

            switch (code)
            {
#pragma region L2CAP_Connection_Request

            case L2CAP_Connection_Request:
            {
                PL2CAP_SIGNALLING_CONNECTION_REQUEST data = (PL2CAP_SIGNALLING_CONNECTION_REQUEST)&buffer[8];

                scid = data->SCID;

                L2CAP_SET_CONNECTION_TYPE(pDeviceContext,
                    pClientDevice,
                    data->PSM,
                    scid,
                    &dcid,
                    0);

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_Connection_Request PSM: %02X SCID: %02X %02X DCID: %02X %02X",
                    data->PSM, scid.Lsb, scid.Msb, dcid.Lsb, dcid.Msb);

                status = L2CAP_Command_Connection_Response(
                    pDeviceContext,
                    pClientDevice->HCI_ConnectionHandle,
                    data->Identifier,
                    scid,
                    dcid,
                    L2CAP_ConnectionResponseResult_ConnectionPending,
                    L2CAP_ConnectionResponseStatus_AuthorisationPending);

                if (!NT_SUCCESS(status))
                {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "L2CAP_Command_Connection_Response (PENDING) failed");
                }

                status = L2CAP_Command_Connection_Response(
                    pDeviceContext,
                    pClientDevice->HCI_ConnectionHandle,
                    data->Identifier,
                    scid,
                    dcid,
                    L2CAP_ConnectionResponseResult_ConnectionSuccessful,
                    L2CAP_ConnectionResponseStatus_NoFurtherInformationAvailable);

                if (!NT_SUCCESS(status))
                {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "L2CAP_Command_Connection_Response (SUCCESSFUL) failed");
                }

                status = L2CAP_Command_Configuration_Request(
                    pDeviceContext,
                    pClientDevice->HCI_ConnectionHandle,
                    pDeviceContext->ClientDeviceList.L2CAP_DataIdentifier++,
                    scid,
                    FALSE);

                if (!NT_SUCCESS(status))
                {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "L2CAP_Command_Configuration_Request failed");
                }

                break;
            }
#pragma endregion

            case L2CAP_Connection_Response:

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_Connection_Response");

                break;
            case L2CAP_Configuration_Request:
            {
                PL2CAP_SIGNALLING_CONFIGURATION_REQUEST data = (PL2CAP_SIGNALLING_CONFIGURATION_REQUEST)&buffer[8];

                dcid = data->DCID;

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_Configuration_Request DCID: %02X %02X",
                    dcid.Lsb, dcid.Msb);

                L2CAP_DEVICE_GET_SCID(pClientDevice, dcid, &scid);

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_Configuration_Request SCID: %02X %02X DCID: %02X %02X",
                    scid.Lsb, scid.Msb, dcid.Lsb, dcid.Msb);

                status = L2CAP_Command_Configuration_Response(
                    pDeviceContext,
                    pClientDevice->HCI_ConnectionHandle,
                    data->Identifier,
                    scid);

                if (!NT_SUCCESS(status))
                {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "L2CAP_Command_Configuration_Response failed");
                }

                if (pClientDevice->IsServiceStarted)
                {
                    pClientDevice->CanStartHid = TRUE;
                    // TODO: action!
                    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "ACTION!");
                }

                break;
            }
            case L2CAP_Configuration_Response:
            {
                PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE data = (PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE)&buffer[8];

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_Configuration_Response 0x%04X", data->Options);

                if (pClientDevice->CanStartService)
                {
                    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "Requesting service connection");

                    L2CAP_Command_Connection_Request(
                        pDeviceContext,
                        pClientDevice->HCI_ConnectionHandle,
                        pDeviceContext->ClientDeviceList.L2CAP_DataIdentifier++,
                        *(PL2CAP_CID)&pDeviceContext->DCID,
                        L2CAP_PSM_HID_Service);

                    pDeviceContext->DCID++;
                }

                break;
            }
            case L2CAP_Disconnection_Request:
            {
                PL2CAP_SIGNALLING_DISCONNECTION_REQUEST data = (PL2CAP_SIGNALLING_DISCONNECTION_REQUEST)&buffer[8];

                scid = data->SCID;
                dcid = data->DCID;

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_Connection_Request SCID: %02X %02X DCID: %02X %02X",
                    scid.Lsb, scid.Msb, dcid.Lsb, dcid.Msb);

                break;
            }
            default:
                TraceEvents(TRACE_LEVEL_WARNING, TRACE_BULKRWR, "Unknown L2CAP command: 0x%02X", code);
                break;
            }
        }
    }
    else if (L2CAP_IS_HID_INPUT_REPORT(buffer))
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_IS_HID_INPUT_REPORT");
    }
    else
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "U.N. Owen");
    }

    //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "%!FUNC! Exit");
}

BOOLEAN
AirBenderEvtUsbBulkReadReadersFailed(
    _In_ WDFUSBPIPE Pipe,
    _In_ NTSTATUS Status,
    _In_ USBD_STATUS UsbdStatus
)
{
    WDFDEVICE device = WdfIoTargetGetDevice(WdfUsbTargetPipeGetIoTarget(Pipe));
    PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(device);

    UNREFERENCED_PARAMETER(pDeviceContext);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "%!FUNC! Entry");

    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "Status: 0x%X, USBD_STATUS: 0x%X", Status, UsbdStatus);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "%!FUNC! Exit");

    return TRUE;
}
