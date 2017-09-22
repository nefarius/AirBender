#pragma once

#define IOCTL_INDEX             0x800
#define FILE_DEVICE_AIRBENDER   65500U

#define IOCTL_AIRBENDER_GET_HOST_BD_ADDR        CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x00, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_READ_ACCESS)

#define IOCTL_AIRBENDER_HOST_RESET              CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x01, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_ANY_ACCESS)

#define IOCTL_AIRBENDER_GET_HID_REPORT          CTL_CODE(FILE_DEVICE_AIRBENDER, \
                                                            IOCTL_INDEX + 0x02, \
                                                            METHOD_BUFFERED,    \
                                                            FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#include <pshpack1.h>

/**
* \typedef struct _BD_ADDR
*
* \brief   Defines a Bluetooth client MAC address.
*/
typedef struct _BD_ADDR
{
    BYTE Address[6];

} BD_ADDR, *PBD_ADDR;

typedef struct _AIRBENDER_GET_HOST_BD_ADDR
{
    BD_ADDR Host;

} AIRBENDER_GET_HOST_BD_ADDR, *PAIRBENDER_GET_HOST_BD_ADDR;

#include <poppack.h>
