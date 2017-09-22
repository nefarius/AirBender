#pragma once

#define DS3_HID_INPUT_REPORT_SIZE   0x31
#define DS3_HID_OUTPUT_REPORT_SIZE  0x32

NTSTATUS
Ds3ConnectionRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PBYTE CID);

NTSTATUS
Ds3ConnectionResponse(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PBYTE CID);

NTSTATUS
Ds3ConfigurationRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer);

NTSTATUS
Ds3ConfigurationResponse(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer,
    PBYTE CID);

NTSTATUS
Ds3DisconnectionRequest(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PUCHAR Buffer);

NTSTATUS
Ds3DisconnectionResponse(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device);

NTSTATUS
Ds3InitHidReportStage(
    PDEVICE_CONTEXT Context,
    PBTH_DEVICE Device,
    PBYTE CID);
