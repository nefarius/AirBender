#pragma once

NTSTATUS
AirBenderConfigContReaderForInterruptEndPoint(
    _In_ PDEVICE_CONTEXT DeviceContext
);

EVT_WDF_USB_READER_COMPLETION_ROUTINE AirBenderEvtUsbInterruptPipeReadComplete;
EVT_WDF_USB_READERS_FAILED AirBenderEvtUsbInterruptReadersFailed;

const __declspec(selectany) LONGLONG DEFAULT_CONTROL_TRANSFER_TIMEOUT = 5 * -1 * WDF_TIMEOUT_TO_SEC;
