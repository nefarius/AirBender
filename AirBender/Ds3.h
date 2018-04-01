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


#pragma once

#define DS3_INIT_HID_STAGE_MAX      0x07
#define DS3_OUTPUT_REPORT_SIZE      0x32

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

NTSTATUS
Ds3ProcessHidInputReport(
    PBTH_DEVICE Device,
    PUCHAR Buffer);
