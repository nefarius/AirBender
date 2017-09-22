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
        INTERRUPT_IN_BUFFER_LENGTH);   // TransferLength

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
            "WdfUsbTargetPipeConfigContinuousReader failed %x\n",
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
    NTSTATUS        status;
    PDEVICE_CONTEXT pDeviceContext = Context;
    PUCHAR          buffer;
    HCI_EVENT       event;
    HCI_COMMAND     command;
    BD_ADDR         clientAddr;
    BTH_HANDLE      clientHandle;

    UNREFERENCED_PARAMETER(Pipe);
    UNREFERENCED_PARAMETER(Buffer);

    if (NumBytesTransferred == 0) {
        TraceEvents(TRACE_LEVEL_WARNING, TRACE_INTERRUPT,
            "!FUNC! Zero length read "
            "occurred on the Interrupt Pipe's Continuous Reader\n"
        );
        return;
    }

    buffer = WdfMemoryGetBuffer(Buffer, NULL);
    event = (HCI_EVENT)buffer[0];
    command = HCI_Null;

    switch (event)
    {
    case HCI_Command_Complete_EV:

        command = (HCI_COMMAND)(USHORT)(buffer[3] | buffer[4] << 8);
        break;
    case HCI_Command_Status_EV:
        //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Command_Status_EV");

        command = (HCI_COMMAND)(USHORT)(buffer[4] | buffer[5] << 8);

        if (buffer[2] != 0)
        {
            switch (command)
            {
            case HCI_Write_Simple_Pairing_Mode:
            case HCI_Write_Authentication_Enable:
            case HCI_Set_Event_Mask:

                pDeviceContext->DisableSSP = TRUE;

                TraceEvents(TRACE_LEVEL_WARNING, TRACE_INTERRUPT,
                    "-- Simple Pairing not supported on this device. [SSP Disabled]");

                status = HCI_Command_Write_Scan_Enable(pDeviceContext);
                break;
            default:
                break;
            }
        }
        break;
    case HCI_Number_Of_Completed_Packets_EV:
        break;
    default:
        break;
    }

    switch (event)
    {
#pragma region HCI_Command_Complete_EV

    case HCI_Command_Complete_EV:

        //TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Command_Complete_EV");

        if (command == HCI_Reset && HCI_COMMAND_SUCCESS(buffer) && !pDeviceContext->Started)
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Reset SUCCESS");

            pDeviceContext->Started = TRUE;

            status = HCI_Command_Read_BD_Addr(pDeviceContext);
        }

        if (command == HCI_Read_BD_ADDR && HCI_COMMAND_SUCCESS(buffer))
        {
            //
            // Reverse byte order to represent MAC address
            // 
            pDeviceContext->BluetoothHostAddress.Address[0] = buffer[11];
            pDeviceContext->BluetoothHostAddress.Address[1] = buffer[10];
            pDeviceContext->BluetoothHostAddress.Address[2] = buffer[9];
            pDeviceContext->BluetoothHostAddress.Address[3] = buffer[8];
            pDeviceContext->BluetoothHostAddress.Address[4] = buffer[7];
            pDeviceContext->BluetoothHostAddress.Address[5] = buffer[6];

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, 
                "HCI_Read_BD_ADDR SUCCESS: %02X:%02X:%02X:%02X:%02X:%02X",
                pDeviceContext->BluetoothHostAddress.Address[0],
                pDeviceContext->BluetoothHostAddress.Address[1],
                pDeviceContext->BluetoothHostAddress.Address[2],
                pDeviceContext->BluetoothHostAddress.Address[3],
                pDeviceContext->BluetoothHostAddress.Address[4],
                pDeviceContext->BluetoothHostAddress.Address[5]);

            status = HCI_Command_Read_Buffer_Size(pDeviceContext);
        }

        if (command == HCI_Read_Buffer_Size && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Read_Buffer_Size SUCCESS");

            status = HCI_Command_Read_Local_Version_Info(pDeviceContext);
        }

        if (command == HCI_Read_Local_Version_Info && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Read_Local_Version_Info SUCCESS");

            BYTE hciMajor = buffer[6];
            BYTE lmpMajor = buffer[9];

            /* analyzes Host Controller Interface (HCI) major version
            * see https://www.bluetooth.org/en-us/specification/assigned-numbers/host-controller-interface
            * */
            switch (hciMajor)
            {
            case 0:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth® Core Specification 1.0b");
                break;
            case 1:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth Core Specification 1.1");
                break;
            case 2:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth Core Specification 1.2");
                break;
            case 3:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth Core Specification 2.0 + EDR");
                break;
            case 4:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth Core Specification 2.1 + EDR");
                break;
            case 5:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth Core Specification 3.0 + HS");
                break;
            case 6:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth Core Specification 4.0");
                break;
            case 7:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth Core Specification 4.1");
                break;
            case 8:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Version: Bluetooth Core Specification 4.2");
                break;
            default:
                break;
            }

            /* analyzes Link Manager Protocol (LMP) major version
            * see https://www.bluetooth.org/en-us/specification/assigned-numbers/link-manager
            * */
            switch (lmpMajor)
            {
            case 0:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth® Core Specification 1.0b");
                break;
            case 1:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth Core Specification 1.1");
                break;
            case 2:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth Core Specification 1.2");
                break;
            case 3:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth Core Specification 2.0 + EDR");
                break;
            case 4:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth Core Specification 2.1 + EDR");
                break;
            case 5:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth Core Specification 3.0 + HS");
                break;
            case 6:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth Core Specification 4.0");
                break;
            case 7:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth Core Specification 4.1");
                break;
            case 8:
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LMP_Version: Bluetooth Core Specification 4.2");
                break;
            default:
                break;
            }

            // Bluetooth v2.0 + EDR
            if (hciMajor >= 3 && lmpMajor >= 3)
            {
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT,
                    "Bluetooth host supports communication with DualShock 3 controllers");
            }

            // Bluetooth v2.1 + EDR
            if (hciMajor >= 4 && lmpMajor >= 4)
            {
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT,
                    "Bluetooth host supports communication with DualShock 4 controllers");
            }

            // dongle effectively too old/unsupported 
            if (hciMajor < 3 || lmpMajor < 3)
            {
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT,
                    "Unsupported Bluetooth Specification, aborting communication");
                status = HCI_Command_Reset(pDeviceContext);
                break;
            }

            if (pDeviceContext->DisableSSP)
            {
                status = HCI_Command_Write_Scan_Enable(pDeviceContext);
            }
            else
            {
                status = HCI_Command_Write_Simple_Pairing_Mode(pDeviceContext);
            }
        }

        if (command == HCI_Write_Simple_Pairing_Mode)
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Simple_Pairing_Mode");

            if (HCI_COMMAND_SUCCESS(buffer))
            {
                status = HCI_Command_Write_Simple_Pairing_Debug_Mode(pDeviceContext);
            }
            else
            {
                pDeviceContext->DisableSSP = TRUE;

                TraceEvents(TRACE_LEVEL_WARNING, TRACE_INTERRUPT, 
                    "-- Simple Pairing not supported on this device. [SSP Disabled]");

                status = HCI_Command_Write_Scan_Enable(pDeviceContext);
            }
        }

        if (command == HCI_Write_Simple_Pairing_Debug_Mode)
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Simple_Pairing_Debug_Mode");

            status = HCI_Command_Write_Authentication_Enable(pDeviceContext);
        }

        if (command == HCI_Write_Authentication_Enable)
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Authentication_Enable");

            if (HCI_COMMAND_SUCCESS(buffer))
            {
                status = HCI_Command_Set_Event_Mask(pDeviceContext);
            }
            else
            {
                pDeviceContext->DisableSSP = TRUE;

                TraceEvents(TRACE_LEVEL_WARNING, TRACE_INTERRUPT,
                    "-- Simple Pairing not supported on this device. [SSP Disabled]");

                status = HCI_Command_Write_Scan_Enable(pDeviceContext);
            }
        }

        if (command == HCI_Set_Event_Mask)
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Set_Event_Mask");

            if (HCI_COMMAND_SUCCESS(buffer))
            {
                status = HCI_Command_Write_Page_Timeout(pDeviceContext);
            }
            else
            {
                pDeviceContext->DisableSSP = TRUE;

                TraceEvents(TRACE_LEVEL_WARNING, TRACE_INTERRUPT,
                    "-- Simple Pairing not supported on this device. [SSP Disabled]");

                status = HCI_Command_Write_Scan_Enable(pDeviceContext);
            }
        }

        if (command == HCI_Write_Page_Timeout && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Page_Timeout");

            status = HCI_Command_Write_Page_Scan_Activity(pDeviceContext);
        }

        if (command == HCI_Write_Page_Scan_Activity && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Page_Scan_Activity");

            status = HCI_Command_Write_Page_Scan_Type(pDeviceContext);
        }

        if (command == HCI_Write_Page_Scan_Type && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Page_Scan_Type");

            status = HCI_Command_Write_Inquiry_Scan_Activity(pDeviceContext);
        }

        if (command == HCI_Write_Inquiry_Scan_Activity && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Inquiry_Scan_Activity");

            status = HCI_Command_Write_Inquiry_Scan_Type(pDeviceContext);
        }

        if (command == HCI_Write_Inquiry_Scan_Type && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Inquiry_Scan_Type");

            status = HCI_Command_Write_Inquiry_Mode(pDeviceContext);
        }

        if (command == HCI_Write_Inquiry_Mode && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Inquiry_Mode");

            status = HCI_Command_Write_Class_of_Device(pDeviceContext);
        }

        if (command == HCI_Write_Class_of_Device && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Class_of_Device");

            status = HCI_Command_Write_Extended_Inquiry_Response(pDeviceContext);
        }

        if (command == HCI_Write_Extended_Inquiry_Response && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Extended_Inquiry_Response");

            status = HCI_Command_Write_Local_Name(pDeviceContext);
        }

        if (command == HCI_Write_Local_Name && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Local_Name");

            status = HCI_Command_Write_Scan_Enable(pDeviceContext);
        }

        if (command == HCI_Write_Scan_Enable && HCI_COMMAND_SUCCESS(buffer))
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Write_Scan_Enable");

            pDeviceContext->Initialized = TRUE;
        }

        break;

#pragma endregion

#pragma region HCI_Connection_Request_EV

    case HCI_Connection_Request_EV:

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT,
            "HCI_Connection_Request_EV %d", (ULONG)NumBytesTransferred);
                
        BD_ADDR_FROM_BUFFER(clientAddr, &buffer[2]);
        BTH_DEVICE_LIST_ADD(&pDeviceContext->ClientDeviceList, &clientAddr);

        status = HCI_Command_Delete_Stored_Link_Key(pDeviceContext, clientAddr);
        status = HCI_Command_Accept_Connection_Request(pDeviceContext, clientAddr, 0x00);

        break;

#pragma endregion

#pragma region HCI_Connection_Complete_EV

    case HCI_Connection_Complete_EV:

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Connection_Complete_EV");

        if (buffer[2] == 0x00)
        {
            clientHandle.Lsb = buffer[3];
            clientHandle.Msb = buffer[4] | 0x20;

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "LSB/MSB: %02X %02X", clientHandle.Lsb, clientHandle.Msb);

            BD_ADDR_FROM_BUFFER(clientAddr, &buffer[5]);

            BTH_DEVICE_LIST_SET_HANDLE(&pDeviceContext->ClientDeviceList, &clientAddr, &clientHandle);

            status = HCI_Command_Remote_Name_Request(pDeviceContext, clientAddr);
        }
        else
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_INTERRUPT, "HCI_Connection_Complete_EV failed: %s", HCI_ERROR_DETAIL(buffer[2]));
        }

        break;

#pragma endregion 

#pragma region HCI_Disconnection_Complete_EV

    case HCI_Disconnection_Complete_EV:

        break;

#pragma endregion

#pragma region HCI_Number_Of_Completed_Packets_EV

    case HCI_Number_Of_Completed_Packets_EV:

        break;

#pragma endregion 

#pragma region HCI_Remote_Name_Request_Complete_EV

    case HCI_Remote_Name_Request_Complete_EV:
    
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Remote_Name_Request_Complete_EV");

        if (buffer[2] == 0x00)
        {
            BD_ADDR_FROM_BUFFER(clientAddr, &buffer[3]);

            PBTH_DEVICE device = BTH_DEVICE_LIST_GET_BY_BD_ADDR(
                &pDeviceContext->ClientDeviceList,
                &clientAddr);

            ULONG length;

            //
            // Scan through rest of buffer until null-terminator is found
            // 
            for (length = 1;
                buffer[length + 8] != 0x00
                && (length + 8) < NumBytesTransferred;
                length++);
               
            //
            // Allocate memory for name (including null-terminator)
            // 
            device->RemoteName = malloc(length);

            //
            // Store remote name in device context
            // 
            RtlCopyMemory(device->RemoteName, &buffer[9], length);

            //
            // Remote name is used to distinguish device type
            // 
            device->DeviceType = 
                (strcmp("Wireless Controller", device->RemoteName) == 0) ? DualShock4 : DualShock3;

            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT,
                "Remote name: %s, length: %d, device: %d", 
                device->RemoteName, length, device->DeviceType);
        }

        break;

#pragma endregion 

#pragma region HCI_Link_Key_Request_EV

    case HCI_Link_Key_Request_EV:

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Link_Key_Request_EV");

        break;

#pragma endregion 

#pragma region HCI_PIN_Code_Request_EV

    case HCI_PIN_Code_Request_EV:

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_PIN_Code_Request_EV");

        break;

#pragma endregion 

#pragma region HCI_IO_Capability_Request_EV

    case HCI_IO_Capability_Request_EV:

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_IO_Capability_Request_EV");

        break;

#pragma endregion

#pragma region HCI_User_Confirmation_Request_EV

    case HCI_User_Confirmation_Request_EV:

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_User_Confirmation_Request_EV");

        break;

#pragma endregion

#pragma region HCI_Link_Key_Notification_EV

    case HCI_Link_Key_Notification_EV:

        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_INTERRUPT, "HCI_Link_Key_Notification_EV");

        break;

#pragma endregion
    default:
        break;
    }
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