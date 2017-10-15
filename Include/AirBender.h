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

#define IOCTL_INDEX             0x801
#define FILE_DEVICE_AIRBENDER   32768U

#define IOCTL_AIRBENDER_GET_HOST_BD_ADDR        CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x00, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_READ_ACCESS)

#define IOCTL_AIRBENDER_HOST_RESET              CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x01, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_ANY_ACCESS)

#define IOCTL_AIRBENDER_GET_CLIENT_COUNT        CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x02, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_READ_ACCESS)

#define IOCTL_AIRBENDER_GET_CLIENT_DETAILS      CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x03, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_AIRBENDER_GET_DS3_INPUT_REPORT    CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x04, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_AIRBENDER_SET_DS3_OUTPUT_REPORT   CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x05, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_WRITE_ACCESS)

#define IOCTL_AIRBENDER_HOST_SHUTDOWN           CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x06, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_ANY_ACCESS)

#include <pshpack1.h>

#define DS3_HID_INPUT_REPORT_SIZE   0x31
#define DS3_HID_OUTPUT_REPORT_SIZE  0x32

/**
* \typedef struct _BD_ADDR
*
* \brief   Defines a Bluetooth client MAC address.
*/
typedef struct _BD_ADDR
{
    BYTE Address[6];

} BD_ADDR, *PBD_ADDR;

/**
* \typedef enum _BTH_DEVICE_TYPE
*
* \brief   Defines an alias representing the possible types of the BTH_DEVICE.
*/
typedef enum _BTH_DEVICE_TYPE
{
    DsTypeUnknown,
    DualShock3,
    DualShock4

} BTH_DEVICE_TYPE;


typedef struct _AIRBENDER_GET_HOST_BD_ADDR
{
    BD_ADDR Host;

} AIRBENDER_GET_HOST_BD_ADDR, *PAIRBENDER_GET_HOST_BD_ADDR;

typedef struct _AIRBENDER_GET_CLIENT_COUNT
{
    ULONG Count;

} AIRBENDER_GET_CLIENT_COUNT, *PAIRBENDER_GET_CLIENT_COUNT;

typedef struct _AIRBENDER_GET_CLIENT_DETAILS
{
    ULONG ClientIndex;

    BTH_DEVICE_TYPE DeviceType;

    BD_ADDR ClientAddress;

} AIRBENDER_GET_CLIENT_DETAILS, *PAIRBENDER_GET_CLIENT_DETAILS;

typedef struct _AIRBENDER_GET_DS3_INPUT_REPORT
{
    BD_ADDR ClientAddress;

    UCHAR ReportBuffer[DS3_HID_INPUT_REPORT_SIZE];

} AIRBENDER_GET_DS3_INPUT_REPORT, *PAIRBENDER_GET_DS3_INPUT_REPORT;

typedef struct _AIRBENDER_SET_DS3_OUTPUT_REPORT
{
    BD_ADDR ClientAddress;

    UCHAR ReportBuffer[DS3_HID_OUTPUT_REPORT_SIZE];

} AIRBENDER_SET_DS3_OUTPUT_REPORT, *PAIRBENDER_SET_DS3_OUTPUT_REPORT;

#include <poppack.h>
