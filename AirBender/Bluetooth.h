#pragma once

#include <stdlib.h>

#define BD_LINK_LENGTH  0x10

static const BYTE BD_LINK[BD_LINK_LENGTH] =
{
    0x56, 0xE8, 0x81, 0x38, 0x08, 0x06, 0x51, 0x41,
    0xC0, 0x7F, 0x12, 0xAA, 0xD9, 0x66, 0x3C, 0xCE
};

/**
 * \typedef struct _BTH_HANDLE
 *
 * \brief   Defines a Bluetooth client handle.
 */
typedef struct _BTH_HANDLE
{
    BYTE Lsb;
    BYTE Msb;

} BTH_HANDLE, *PBTH_HANDLE;

/**
 * \typedef struct _BTH_HANDLE_PAIR
 *
 * \brief   Defines a handle pair connecting a device CID to a host CID.
 */
typedef struct _BTH_HANDLE_PAIR
{
    BTH_HANDLE Source;
    BTH_HANDLE Destination;

} BTH_HANDLE_PAIR;

typedef struct _BTH_DEVICE_HID_REPORT
{
    LPVOID Data;
    ULONG Length;

} BTH_DEVICE_HID_REPORT, *PBTH_DEVICE_HID_REPORT;

/**
 * \typedef struct _BTH_DEVICE
 *
 * \brief   Defines a Bluetooth client device connection information set.
 */
typedef struct _BTH_DEVICE
{
    BD_ADDR ClientAddress;

    BTH_HANDLE HCI_ConnectionHandle;

    BTH_HANDLE_PAIR L2CAP_CommandHandle;

    BTH_HANDLE_PAIR L2CAP_InterruptHandle;

    BTH_HANDLE_PAIR L2CAP_ServiceHandle;

    BOOLEAN CanStartService;

    BOOLEAN IsServiceStarted;

    BOOLEAN CanStartHid;

    BYTE InitHidStage;

    LPSTR RemoteName;

    BTH_DEVICE_TYPE DeviceType;

    WDFQUEUE HidInputReportQueue;

    struct _BTH_DEVICE *next;

} BTH_DEVICE, *PBTH_DEVICE;

/**
 * \typedef struct _BTH_DEVICE_LIST
 *
 * \brief   Defines a linked list of Bluetooth client devices.
 */
typedef struct _BTH_DEVICE_LIST
{
    ULONG logicalLength;

    PBTH_DEVICE head;

    PBTH_DEVICE tail;

} BTH_DEVICE_LIST, *PBTH_DEVICE_LIST;

#define BD_ADDR_FROM_BUFFER(_addr_, _buf_)      (RtlCopyMemory(&_addr_, _buf_, sizeof(BD_ADDR)));

/**
 * \def BTH_HANDLE_FROM_BUFFER(_ch_, _buf_) (RtlCopyMemory(&_ch_, _buf_, sizeof(BTH_HANDLE)));
 *
 * \brief   A macro that extracts a BTH_HANDLE from a bulk input pipe buffer.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    19.09.2017
 *
 * \param   _ch_    The target client handle.
 * \param   _buf_   The buffer.
 */
#define BTH_HANDLE_FROM_BUFFER(_ch_, _buf_)     (RtlCopyMemory(&_ch_, _buf_, sizeof(BTH_HANDLE)));

 /**
  * \fn  VOID FORCEINLINE BTH_DEVICE_FREE( PBTH_DEVICE Device )
  *
  * \brief   Frees resources allocated by provided BTH_DEVICE.
  *
  * \author  Benjamin "Nefarius" Höglinger
  * \date    20.09.2017
  *
  * \param   Device  The BTH_DEVICE handle.
  *
  * \return  Nothing.
  */
VOID FORCEINLINE BTH_DEVICE_FREE(
    PBTH_DEVICE Device
)
{
    if (Device->RemoteName)
        free(Device->RemoteName);

    WdfIoQueuePurgeSynchronously(Device->HidInputReportQueue);
    WdfObjectDelete(Device->HidInputReportQueue);
}

/**
 * \fn  VOID FORCEINLINE BTH_DEVICE_LIST_INIT( PBTH_DEVICE_LIST List )
 *
 * \brief   Initializes a new Bluetooth client device list.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    15.09.2017
 *
 * \param   List    The device list.
 *
 * \return  Nothing.
 */
VOID FORCEINLINE BTH_DEVICE_LIST_INIT(
    PBTH_DEVICE_LIST List
)
{
    List->logicalLength = 0;
    List->head = List->tail = NULL;
}

/**
 * \fn  VOID FORCEINLINE BTH_DEVICE_LIST_ADD( PBTH_DEVICE_LIST List, PBD_ADDR Address )
 *
 * \brief   Adds a new device to the list identified by the clients MAC address.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    15.09.2017
 *
 * \param   List    The device list.
 * \param   Address The client MAC address.
 *
 * \return  NTSTATUS value.
 */
NTSTATUS FORCEINLINE BTH_DEVICE_LIST_ADD(
    PBTH_DEVICE_LIST List,
    PBD_ADDR Address,
    WDFDEVICE HostDevice
)
{
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueCfg;
    PBTH_DEVICE node = malloc(sizeof(BTH_DEVICE));
    RtlZeroMemory(node, sizeof(BTH_DEVICE));

    node->ClientAddress = *Address;

    WDF_IO_QUEUE_CONFIG_INIT(&queueCfg, WdfIoQueueDispatchManual);
    status = WdfIoQueueCreate(HostDevice, &queueCfg, WDF_NO_OBJECT_ATTRIBUTES, &node->HidInputReportQueue);
    if (!NT_SUCCESS(status)) {
        free(node);
        return status;
    }

    if (List->logicalLength == 0) {
        List->head = List->tail = node;
    }
    else {
        List->tail->next = node;
        List->tail = node;
    }

    List->logicalLength++;

    return status;
}

/**
 * \fn  ULONG FORCEINLINE BTH_DEVICE_LIST_GET_COUNT( PBTH_DEVICE_LIST List )
 *
 * \brief   Returns the element count of the list.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    15.09.2017
 *
 * \param   List    The device list.
 *
 * \return  The element count of the list.
 */
ULONG FORCEINLINE BTH_DEVICE_LIST_GET_COUNT(
    PBTH_DEVICE_LIST List
)
{
    return List->logicalLength;
}

/**
 * \fn  PBTH_DEVICE FORCEINLINE BTH_DEVICE_LIST_GET_BY_BD_ADDR( PBTH_DEVICE_LIST List, PBD_ADDR Address )
 *
 * \brief   Returns a list element identified by the provided MAC address.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    15.09.2017
 *
 * \param   List    The device list.
 * \param   Address The client MAC address.
 *
 * \return  A pointer to the list element, NULL if not found.
 */
PBTH_DEVICE FORCEINLINE BTH_DEVICE_LIST_GET_BY_BD_ADDR(
    PBTH_DEVICE_LIST List,
    PBD_ADDR Address
)
{
    PBTH_DEVICE node = List->head;

    while (node != NULL)
    {
        if (RtlCompareMemory(
            &node->ClientAddress,
            Address,
            sizeof(BD_ADDR)) == sizeof(BD_ADDR))
        {
            return node;
        }

        node = node->next;
    }

    return NULL;
}

/**
 * \fn  VOID FORCEINLINE BTH_DEVICE_LIST_SET_HANDLE( PBTH_DEVICE_LIST List, PBD_ADDR Address, PBTH_HANDLE Handle )
 *
 * \brief   Sets the device handle value for a list element identified by the provided client MAC
 *          address.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    15.09.2017
 *
 * \param   List    The device list.
 * \param   Address The client MAC address.
 * \param   Handle  The client device handle.
 *
 * \return  Nothing.
 */
VOID FORCEINLINE BTH_DEVICE_LIST_SET_HANDLE(
    PBTH_DEVICE_LIST List,
    PBD_ADDR Address,
    PBTH_HANDLE Handle
)
{
    PBTH_DEVICE node = BTH_DEVICE_LIST_GET_BY_BD_ADDR(List, Address);

    node->HCI_ConnectionHandle = *Handle;
}

/**
 * \fn  PBTH_DEVICE FORCEINLINE BTH_DEVICE_LIST_GET_BY_HANDLE( PBTH_DEVICE_LIST List, PBTH_HANDLE Handle )
 *
 * \brief   Returns a list element identified by the provided client handle.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    15.09.2017
 *
 * \param   List    The device list.
 * \param   Handle  The client handle.
 *
 * \return  A pointer to the list element, NULL if not found.
 */
PBTH_DEVICE FORCEINLINE BTH_DEVICE_LIST_GET_BY_HANDLE(
    PBTH_DEVICE_LIST List,
    PBTH_HANDLE Handle
)
{
    PBTH_DEVICE node = List->head;

    while (node != NULL)
    {
        if (RtlCompareMemory(
            &node->HCI_ConnectionHandle,
            Handle,
            sizeof(BTH_HANDLE)) == sizeof(BTH_HANDLE))
        {
            return node;
        }

        node = node->next;
    }

    return NULL;
}

/**
 * \fn  VOID FORCEINLINE BTH_DEVICE_LIST_FREE( PBTH_DEVICE_LIST List )
 *
 * \brief   Cleans the device list and disposes resources allocated by all children.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    22.09.2017
 *
 * \param   List    The device list.
 *
 * \return  Nothing.
 */
VOID FORCEINLINE BTH_DEVICE_LIST_FREE(
    PBTH_DEVICE_LIST List
)
{
    PBTH_DEVICE node = List->head;

    while (node != NULL)
    {
        BTH_DEVICE_FREE(node);

        node = node->next;
    }

    RtlZeroMemory(List, sizeof(BTH_DEVICE_LIST));
}

/**
 * \fn  PBTH_DEVICE FORCEINLINE BTH_DEVICE_LIST_GET_BY_INDEX( PBTH_DEVICE_LIST List, ULONG Index )
 *
 * \brief   Returns a list element identified by the provided node index.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    24.09.2017
 *
 * \param   List    The device list.
 * \param   Index   Zero-based index of the desired node.
 *
 * \return  A FORCEINLINE.
 */
PBTH_DEVICE FORCEINLINE BTH_DEVICE_LIST_GET_BY_INDEX(
    PBTH_DEVICE_LIST List,
    ULONG Index
)
{
    PBTH_DEVICE node = List->head;
    ULONG i = 0;

    while (node != NULL)
    {
        if (i++ == Index) return node;
    }

    return NULL;
}

