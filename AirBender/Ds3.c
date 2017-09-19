#include "Driver.h"

#include "ds3.tmh"
#include "L2CAP.h"

NTSTATUS
Ds3ConnectionRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PBYTE CID)
{
    NTSTATUS    status;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;

    PL2CAP_SIGNALLING_CONNECTION_REQUEST data = (PL2CAP_SIGNALLING_CONNECTION_REQUEST)&Buffer[8];

    scid = data->SCID;

    L2CAP_SET_CONNECTION_TYPE(
        Device,
        data->PSM,
        scid,
        &dcid);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "! L2CAP_SET_CONNECTION_TYPE: PSM: %02X SCID: %04X DCID: %04X",
        data->PSM, *(PUSHORT)&scid, *(PUSHORT)&dcid);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Connection_Request PSM: %02X SCID: %04X DCID: %04X",
        data->PSM, *(PUSHORT)&scid, *(PUSHORT)&dcid);

    status = L2CAP_Command_Connection_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        scid,
        dcid,
        L2CAP_ConnectionResponseResult_ConnectionPending,
        L2CAP_ConnectionResponseStatus_AuthorisationPending);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Connection_Response (PENDING) failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Connection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    status = L2CAP_Command_Connection_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        scid,
        dcid,
        L2CAP_ConnectionResponseResult_ConnectionSuccessful,
        L2CAP_ConnectionResponseStatus_NoFurtherInformationAvailable);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Connection_Response (SUCCESSFUL) failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Connection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    status = L2CAP_Command_Configuration_Request(
        Context,
        Device->HCI_ConnectionHandle,
        (*CID)++,
        scid,
        TRUE);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Configuration_Request failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Configuration_Request SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    return status;
}

NTSTATUS
Ds3ConnectionResponse(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PBYTE CID)
{
    NTSTATUS    status = STATUS_SUCCESS;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;

    PL2CAP_SIGNALLING_CONNECTION_RESPONSE data = (PL2CAP_SIGNALLING_CONNECTION_RESPONSE)&Buffer[8];

    scid = data->SCID;
    dcid = data->DCID;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Connection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    switch ((L2CAP_CONNECTION_RESPONSE_RESULT)data->Result)
    {
    case L2CAP_ConnectionResponseResult_ConnectionSuccessful:

        L2CAP_SET_CONNECTION_TYPE(
            Device,
            L2CAP_PSM_HID_Service,
            dcid,
            &scid);

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "! L2CAP_SET_CONNECTION_TYPE: L2CAP_PSM_HID_Service SCID: %04X DCID: %04X",
            *(PUSHORT)&scid, *(PUSHORT)&dcid);

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            ">> >> L2CAP_ConnectionResponseResult_ConnectionSuccessful SCID: %04X DCID: %04X",
            *(PUSHORT)&scid, *(PUSHORT)&dcid);

        status = L2CAP_Command_Configuration_Request(
            Context,
            Device->HCI_ConnectionHandle,
            (*CID)++,
            dcid,
            TRUE);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
                "L2CAP_Command_Configuration_Request failed");
            break;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< L2CAP_Configuration_Request SCID: %04X DCID: %04X",
            *(PUSHORT)&scid, *(PUSHORT)&dcid);

        break;
    case L2CAP_ConnectionResponseResult_ConnectionPending:
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            ">> >> L2CAP_ConnectionResponseResult_ConnectionPending");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported:
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock:
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock");
        break;
    case L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable:
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3,
            "L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable");
        break;
    default:
        break;
    }

    return status;
}

NTSTATUS Ds3ConfigurationRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    NTSTATUS    status;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;
    PVOID       pHidCmd;
    ULONG       hidCmdLen;

    PL2CAP_SIGNALLING_CONFIGURATION_REQUEST data = (PL2CAP_SIGNALLING_CONFIGURATION_REQUEST)&Buffer[8];

    dcid = data->DCID;

    L2CAP_DEVICE_GET_SCID(Device, dcid, &scid);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "! L2CAP_DEVICE_GET_SCID: DCID %04X -> SCID %04X",
        *(PUSHORT)&dcid, *(PUSHORT)&scid);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Configuration_Request SCID: %04X DCID: %04X",
        *(PUSHORT)&scid.Msb, *(PUSHORT)&dcid);

    status = L2CAP_Command_Configuration_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        scid);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Configuration_Response failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Configuration_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    if (Device->IsServiceStarted)
    {
        Device->CanStartHid = TRUE;

        if (Device->InitHidStage < 0x07)
        {
            L2CAP_DEVICE_GET_SCID_FOR_TYPE(
                Device,
                L2CAP_PSM_HID_Service,
                &scid);

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
                "! L2CAP_DEVICE_GET_SCID_FOR_TYPE: L2CAP_PSM_HID_Service -> SCID %04X",
                *(PUSHORT)&scid);

            GetElementsByteArray(
                &Context->HidInitReports,
                Device->InitHidStage++,
                &pHidCmd,
                &hidCmdLen);

            status = HID_Command(
                Context,
                Device->HCI_ConnectionHandle,
                scid,
                pHidCmd,
                hidCmdLen);

            if (!NT_SUCCESS(status))
            {
                TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "HID_Command failed");
                return status;
            }

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
                "<< HID_Command Index: %d, Length: %d",
                Device->InitHidStage - 1, hidCmdLen);
        }
    }

    return status;
}

NTSTATUS
Ds3ConfigurationResponse(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PBYTE CID)
{
    NTSTATUS    status = STATUS_SUCCESS;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;

    PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE data = (PL2CAP_SIGNALLING_CONFIGURATION_RESPONSE)&Buffer[8];

    scid = data->SCID;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Configuration_Response SCID: 0x%04X",
        *(PUSHORT)&scid);

    if (Device->CanStartService)
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3, "Requesting service connection");

        L2CAP_GET_NEW_CID(&dcid);

        status = L2CAP_Command_Connection_Request(
            Context,
            Device->HCI_ConnectionHandle,
            (*CID)++,
            dcid,
            L2CAP_PSM_HID_Service);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Connection_Request failed");
            return status;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< L2CAP_Connection_Request SCID: %04X DCID: %04X",
            *(PUSHORT)&scid, *(PUSHORT)&dcid);
    }

    return status;
}

NTSTATUS
Ds3DisconnectionRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer)
{
    NTSTATUS    status;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;

    PL2CAP_SIGNALLING_DISCONNECTION_REQUEST data = (PL2CAP_SIGNALLING_DISCONNECTION_REQUEST)&Buffer[8];

    scid = data->SCID;
    dcid = data->DCID;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Disconnection_Request SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    status = L2CAP_Command_Disconnection_Response(
        Context,
        Device->HCI_ConnectionHandle,
        data->Identifier,
        dcid,
        scid);

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Disconnection_Response failed");
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        "<< L2CAP_Disconnection_Response SCID: %04X DCID: %04X",
        *(PUSHORT)&scid, *(PUSHORT)&dcid);

    return status;
}

NTSTATUS
Ds3DisconnectionResponse(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device)
{
    NTSTATUS    status = STATUS_SUCCESS;
    L2CAP_CID   scid;
    BYTE        hidCommandEnable[] = { 0x53, 0xF4, 0x42, 0x03, 0x00, 0x00 };

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
        ">> L2CAP_Disconnection_Response");

    if (Device->CanStartHid)
    {
        Device->IsServiceStarted = FALSE;

        L2CAP_DEVICE_GET_SCID_FOR_TYPE(
            Device,
            L2CAP_PSM_HID_Command,
            &scid);

        status = HID_Command(
            Context,
            Device->HCI_ConnectionHandle,
            scid,
            hidCommandEnable,
            _countof(hidCommandEnable));

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "HID_Command failed");
            return status;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< HID_Command ENABLE sent");
    }

    return status;
}

NTSTATUS
Ds3InitHidReportStage(
    PDEVICE_CONTEXT Context, 
    PBTH_DEVICE Device, 
    PBYTE CID)
{
    NTSTATUS    status = STATUS_SUCCESS;
    L2CAP_CID   dcid;
    L2CAP_CID   scid;
    PVOID       pHidCmd;
    ULONG       hidCmdLen;

    if (Device->InitHidStage < 0x07)
    {
        L2CAP_DEVICE_GET_SCID_FOR_TYPE(
            Device,
            L2CAP_PSM_HID_Service,
            &scid);

        GetElementsByteArray(
            &Context->HidInitReports,
            Device->InitHidStage++,
            &pHidCmd,
            &hidCmdLen);

        status = HID_Command(
            Context,
            Device->HCI_ConnectionHandle,
            scid,
            pHidCmd,
            hidCmdLen);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "HID_Command failed");
            return status;
        }

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< HID_Command Index: %d, Length: %d",
            Device->InitHidStage - 1, hidCmdLen);
    }
    else if(Device->IsServiceStarted)
    {
        L2CAP_DEVICE_GET_SCID_FOR_TYPE(Device, L2CAP_PSM_HID_Service, &scid);
        L2CAP_DEVICE_GET_DCID_FOR_TYPE(Device, L2CAP_PSM_HID_Service, &dcid);

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DS3,
            "<< L2CAP_Disconnection_Request SCID: %04X DCID: %04X",
            *(PUSHORT)&scid, *(PUSHORT)&dcid);

        status = L2CAP_Command_Disconnection_Request(
            Context,
            Device->HCI_ConnectionHandle,
            (*CID)++,
            scid,
            dcid);

        if (!NT_SUCCESS(status))
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DS3, "L2CAP_Command_Disconnection_Request failed");
        }
    }

    return status;
}
