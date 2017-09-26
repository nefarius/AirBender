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
