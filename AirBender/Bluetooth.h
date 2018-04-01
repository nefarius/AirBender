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

#include <stdlib.h>

#define BD_LINK_LENGTH  0x10
#define DS3_OUTPUT_REPORT_TIMER_PERIOD     10 // ms

EVT_WDF_TIMER AirBenderBulkWriteEvtTimerFunc;


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

/**
 * \typedef struct _BTH_DEVICE
 *
 * \brief   Defines a Bluetooth client device connection information set.
 */
typedef struct _BTH_DEVICE
{
    //
    // MAC address identifying this device
    // 
    BD_ADDR ClientAddress;

    //
    // Handle identifying the parent host controller of this device
    // 
    BTH_HANDLE HCI_ConnectionHandle;

    //
    // Handle identifying the L2CAP command channel
    // 
    BTH_HANDLE_PAIR L2CAP_CommandHandle;

    //
    // Handle identifying the L2CAP interrupt channel
    // 
    BTH_HANDLE_PAIR L2CAP_InterruptHandle;

    //
    // Handle identifying the L2CAP service channel
    // 
    BTH_HANDLE_PAIR L2CAP_ServiceHandle;

    //
    // Indicates if the service channel can start
    // 
    BOOLEAN CanStartService;

    //
    // Indicates if the service channel is ready
    // 
    BOOLEAN IsServiceStarted;

    //
    // Indicates if the HID channel can start
    // 
    BOOLEAN CanStartHid;

    //
    // Index of the current HID initialization packet
    // 
    BYTE InitHidStage;

    //
    // Name reported by this device
    // 
    LPSTR RemoteName;

    //
    // Controller type
    // 
    BTH_DEVICE_TYPE DeviceType;

    //
    // Framework queue storing HID input requests
    // 
    WDFQUEUE HidInputReportQueue;

    //
    // Framework memory holding output repor
    // 
    WDFMEMORY HidOutputReportMemory;

    //
    // Framework object for periodic output timer
    // 
    WDFTIMER HidOutputReportTimer;

    //
    // Pointer to next device in the list
    // 
    struct _BTH_DEVICE *next;

} BTH_DEVICE, *PBTH_DEVICE;

/**
 * \struct  _BTH_DEVICE_CONTEXT
 *
 * \brief   A Bluetooth device context.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    27.03.2018
 */
typedef struct _BTH_DEVICE_CONTEXT
{
    PBTH_DEVICE Device;

    WDFDEVICE HostDevice;

} BTH_DEVICE_CONTEXT, *PBTH_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(BTH_DEVICE_CONTEXT, BluetoothDeviceGetContext)

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
 * \fn  NTSTATUS FORCEINLINE BTH_DEVICE_LIST_ADD( PBTH_DEVICE_LIST List, PBD_ADDR Address, WDFDEVICE HostDevice )
 *
 * \brief   Adds a new device to the list identified by the clients MAC address.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    15.09.2017
 *
 * \param   List        The device list.
 * \param   Address     The client MAC address.
 * \param   HostDevice  The host device.
 *
 * \return  NTSTATUS value.
 */
NTSTATUS FORCEINLINE BTH_DEVICE_LIST_ADD(
    PBTH_DEVICE_LIST List,
    PBD_ADDR Address,
    WDFDEVICE HostDevice
)
{
    NTSTATUS                status;
    WDF_IO_QUEUE_CONFIG     queueCfg;
    WDF_TIMER_CONFIG        timerCfg;
    WDF_OBJECT_ATTRIBUTES   attributes;
    PBTH_DEVICE_CONTEXT     pBluetoothCtx;
    PBTH_DEVICE node = malloc(sizeof(BTH_DEVICE));
    RtlZeroMemory(node, sizeof(BTH_DEVICE));

    node->ClientAddress = *Address;

    //
    // Create queue for input reports
    // 
    WDF_IO_QUEUE_CONFIG_INIT(&queueCfg, WdfIoQueueDispatchManual);
    status = WdfIoQueueCreate(HostDevice, &queueCfg, WDF_NO_OBJECT_ATTRIBUTES, &node->HidInputReportQueue);
    if (!NT_SUCCESS(status)) {
        free(node);
        return status;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = node->HidInputReportQueue;

    //
    // Create periodic timer for output report
    // 
    WDF_TIMER_CONFIG_INIT_PERIODIC(
        &timerCfg,
        AirBenderBulkWriteEvtTimerFunc,
        DS3_OUTPUT_REPORT_TIMER_PERIOD
    );
    status = WdfTimerCreate(&timerCfg, &attributes, &node->HidOutputReportTimer);
    if (!NT_SUCCESS(status)) {
        free(node);
        return status;
    }

    //
    // Allocate context for timer to tie it together with the BTH_DEVICE
    // 
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, BTH_DEVICE_CONTEXT);
    status = WdfObjectAllocateContext(
        node->HidOutputReportTimer,
        &attributes,
        (PVOID)&pBluetoothCtx
    );
    if (!NT_SUCCESS(status)) {
        free(node);
        return status;
    }

    pBluetoothCtx->Device = node;
    pBluetoothCtx->HostDevice = HostDevice;

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
 * \fn  BOOLEAN FORCEINLINE BTH_DEVICE_LIST_REMOVE( PBTH_DEVICE_LIST List, PBTH_HANDLE Handle )
 *
 * \brief   Removes a new device from the list identified by its handle.
 *
 * \author  Benjamin "Nefarius" Höglinger
 * \date    03.11.2017
 *
 * \param   List    The device list.
 * \param   Handle  The client handle.
 *
 * \return  TRUE if the provided device got removed from the list, FALSE otherwise.
 */
BOOLEAN FORCEINLINE BTH_DEVICE_LIST_REMOVE(
    PBTH_DEVICE_LIST List,
    PBTH_HANDLE Handle
)
{
    BTH_DEVICE *currP, *prevP;

    /* For 1st node, indicate there is no previous. */
    prevP = NULL;

    /*
    * Visit each node, maintaining a pointer to
    * the previous node we just visited.
    */
    for (currP = List->head; currP != NULL; prevP = currP, currP = currP->next)
    {
        if (*(PUSHORT)&currP->HCI_ConnectionHandle == *(PUSHORT)Handle)
        {
            /* Found it. */
            if (prevP == NULL)
            {
                /* Fix beginning pointer. */
                List->head = currP->next;
            }
            else
            {
                /*
                * Fix previous node's next to
                * skip over the removed node.
                */
                prevP->next = currP->next;
            }

            /* Deallocate the node. */
            free(currP);

            List->logicalLength--;

            /* Done searching. */
            return TRUE;
        }
    }

    return FALSE;
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

