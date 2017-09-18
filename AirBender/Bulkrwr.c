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

NTSTATUS 
HID_Command(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    L2CAP_CID Channel,
    PVOID Buffer,
    ULONG BufferLength
)
{
    NTSTATUS status;
    PUCHAR buffer = malloc(BufferLength + 8);

    buffer[0] = Handle.Lsb;
    buffer[1] = Handle.Msb;
    buffer[2] = (BYTE)((BufferLength + 4) % 256);
    buffer[3] = (BYTE)((BufferLength + 4) / 256);
    buffer[4] = (BYTE)(BufferLength % 256);
    buffer[5] = (BYTE)(BufferLength / 256);
    buffer[6] = Channel.Lsb;
    buffer[7] = Channel.Msb;

    RtlCopyMemory(&buffer[8], Buffer, BufferLength);

    status = WriteBulkPipe(Context, buffer, BufferLength + 8, NULL);

    free(buffer);

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
    
    static BYTE CID = 0x01;

    UNREFERENCED_PARAMETER(Pipe);

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

    if (pClientDevice == NULL)
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "PBTH_DEVICE not found");
        return;
    }

    if (L2CAP_IS_CONTROL_CHANNEL(buffer))
    {
        //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_IS_CONTROL_CHANNEL");

        if (L2CAP_IS_SIGNALLING_COMMAND_CODE(buffer))
        {
            code = L2CAP_GET_SIGNALLING_COMMAND_CODE(buffer);

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_IS_SIGNALLING_COMMAND_CODE: 0x%02X", (BYTE)code);

            switch (code)
            {
            case L2CAP_Command_Reject:
            {
                PL2CAP_SIGNALLING_COMMAND_REJECT data = (PL2CAP_SIGNALLING_COMMAND_REJECT)&buffer[8];

                TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "L2CAP_Command_Reject: 0x%04X", data->Reason);

                break;
            }

#pragma region L2CAP_Connection_Request

            case L2CAP_Connection_Request:
            {
                PL2CAP_SIGNALLING_CONNECTION_REQUEST data = (PL2CAP_SIGNALLING_CONNECTION_REQUEST)&buffer[8];

                scid = data->SCID;

                L2CAP_SET_CONNECTION_TYPE(
                    pClientDevice,
                    data->PSM,
                    scid,
                    &dcid);

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, 
                    "[CID: 0x%02X] L2CAP_Connection_Request PSM: %02X SCID: %04X DCID: %04X",
                    data->Identifier, data->PSM, *(PUSHORT)&scid, *(PUSHORT)&dcid);

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
                    CID++,
                    scid,
                    TRUE);

                if (!NT_SUCCESS(status))
                {
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "L2CAP_Command_Configuration_Request failed");
                }

                break;
            }
#pragma endregion

#pragma region L2CAP_Connection_Response

            case L2CAP_Connection_Response:
            {
                PL2CAP_SIGNALLING_CONNECTION_RESPONSE data = (PL2CAP_SIGNALLING_CONNECTION_RESPONSE)&buffer[8];

                scid = data->SCID;
                dcid = data->DCID;

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR,
                    "[CID: 0x%02X] L2CAP_Connection_Response SCID: %04X DCID: %04X",
                    data->Identifier, *(PUSHORT)&scid, *(PUSHORT)&dcid);

                switch ((L2CAP_CONNECTION_RESPONSE_RESULT)data->Result)
                {
                case L2CAP_ConnectionResponseResult_ConnectionSuccessful:

                    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_ConnectionResponseResult_ConnectionSuccessful SCID: 0x%04X", 
                        *(PUSHORT)&scid);

                    L2CAP_SET_CONNECTION_TYPE(
                        pClientDevice,
                        L2CAP_PSM_HID_Service,
                        scid,
                        &dcid);

                    status = L2CAP_Command_Configuration_Request(
                        pDeviceContext,
                        pClientDevice->HCI_ConnectionHandle,
                        CID++,
                        dcid,
                        TRUE);

                    if (!NT_SUCCESS(status))
                    {
                        TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR, "L2CAP_Command_Configuration_Request failed");
                    }

                    break;
                case L2CAP_ConnectionResponseResult_ConnectionPending:
                    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR,
                        "L2CAP_ConnectionResponseResult_ConnectionPending");
                    break;
                case L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported:
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
                        "L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported");
                    break;
                case L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock:
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
                        "L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock");
                    break;
                case L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable:
                    TraceEvents(TRACE_LEVEL_ERROR, TRACE_BULKRWR,
                        "L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable");
                    break;
                default:
                    break;
                }

                break;
            }
#pragma endregion

#pragma region L2CAP_Configuration_Request

            case L2CAP_Configuration_Request:
            {
                PL2CAP_SIGNALLING_CONFIGURATION_REQUEST data = (PL2CAP_SIGNALLING_CONFIGURATION_REQUEST)&buffer[8];

                dcid = data->DCID;

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, 
                    "[CID: 0x%02X] L2CAP_Configuration_Request DCID: %04X",
                    data->Identifier, *(PUSHORT)&dcid.Lsb);

                L2CAP_DEVICE_GET_SCID(pClientDevice, dcid, &scid);

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "L2CAP_Configuration_Request SCID: %04X DCID: %04X",
                    *(PUSHORT)&scid.Msb, *(PUSHORT)&dcid);

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
#pragma endregion

#pragma region L2CAP_Configuration_Response

            case L2CAP_Configuration_Response:
            {
                PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE data = (PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE)&buffer[8];

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, 
                    "[CID: 0x%02X] L2CAP_Configuration_Response 0x%04X", 
                    data->Identifier, data->Options);

                if (pClientDevice->CanStartService)
                {
                    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR, "Requesting service connection");

                    L2CAP_GET_NEW_CID(&dcid);

                    L2CAP_Command_Connection_Request(
                        pDeviceContext,
                        pClientDevice->HCI_ConnectionHandle,
                        CID++,
                        dcid,
                        L2CAP_PSM_HID_Service);
                }

                break;
            }
#pragma endregion

            case L2CAP_Disconnection_Request:
            {
                PL2CAP_SIGNALLING_DISCONNECTION_REQUEST data = (PL2CAP_SIGNALLING_DISCONNECTION_REQUEST)&buffer[8];

                scid = data->SCID;
                dcid = data->DCID;

                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_BULKRWR,
                    "[CID: 0x%02X] L2CAP_Disconnection_Request SCID: %04X DCID: %04X",
                    data->Identifier, *(PUSHORT)&scid, *(PUSHORT)&dcid);

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
