#pragma once

#define IOCTL_INDEX             0x800
#define FILE_DEVICE_AIRBENDER   65500U

#define IOCTL_AIRBENDER_CHILDREN_GET_COUNT      CTL_CODE(FILE_DEVICE_AIRBENDER  \
                                                            IOCTL_INDEX,        \
                                                            METHOD_BUFFERED,    \
                                                            FILE_READ_ACCESS)

#define IOCTL_AIRBENDER_GET_HID_REPORT          CTL_CODE(FILE_DEVICE_AIRBENDER  \
                                                            IOCTL_INDEX,        \
                                                            METHOD_BUFFERED,    \
                                                            FILE_READ_ACCESS | FILE_WRITE_ACCESS)
