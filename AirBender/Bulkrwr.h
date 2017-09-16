#pragma once

NTSTATUS
AirBenderConfigContReaderForBulkReadEndPoint(
    _In_ PDEVICE_CONTEXT DeviceContext
);

EVT_WDF_USB_READER_COMPLETION_ROUTINE AirBenderEvtUsbBulkReadPipeReadComplete;
EVT_WDF_USB_READERS_FAILED AirBenderEvtUsbBulkReadReadersFailed;

#define BULK_IN_BUFFER_LENGTH  512

NTSTATUS
WriteBulkPipe(
    _In_ PDEVICE_CONTEXT Context,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength,
    _Out_opt_ PULONG BytesWritten
);
