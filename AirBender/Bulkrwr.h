#pragma once

NTSTATUS
AirBenderConfigContReaderForBulkReadEndPoint(
    _In_ PDEVICE_CONTEXT DeviceContext
);

EVT_WDF_USB_READER_COMPLETION_ROUTINE AirBenderEvtUsbBulkReadPipeReadComplete;
EVT_WDF_USB_READERS_FAILED AirBenderEvtUsbBulkReadReadersFailed;

#define BULK_IN_BUFFER_LENGTH  512
