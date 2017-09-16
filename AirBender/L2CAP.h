#pragma once

typedef enum _L2CAP_SIGNALLING_COMMAND_CODE
{
    L2CAP_Reserved = 0x00,
    L2CAP_Command_Reject = 0x01,

    /// <summary>
    ///     A Connection Request packet has been received.
    /// </summary>
    L2CAP_Connection_Request = 0x02,

    /// <summary>
    ///     A Connection Response packet has been received with a positive result indicating that the connection has been
    ///     established.
    /// </summary>
    L2CAP_Connection_Response = 0x03,

    /// <summary>
    ///     A Configuration Request packet has been received indicating the remote endpoint wishes to engage in negotiations
    ///     concerning channel parameters.
    /// </summary>
    L2CAP_Configuration_Request = 0x04,

    /// <summary>
    ///     A Configuration Response packet has been received indicating the remote endpoint agrees with all the parameters
    ///     being negotiated.
    /// </summary>
    L2CAP_Configuration_Response = 0x05,

    /// <summary>
    ///     A Disconnection Request packet has been received and the channel must initiate the disconnection process. Following
    ///     the completion of an L2CAP channel disconnection process, an L2CAP entity should return the corresponding local CID
    ///     to the pool of “unassigned” CIDs.
    /// </summary>
    L2CAP_Disconnection_Request = 0x06,

    /// <summary>
    ///     A Disconnection Response packet has been received. Following the receipt of this signal, the receiving L2CAP entity
    ///     may return the corresponding local CID to the pool of unassigned CIDs. There is no corresponding negative response
    ///     because the Disconnect Request must succeed.
    /// </summary>
    L2CAP_Disconnection_Response = 0x07,
    L2CAP_Echo_Request = 0x08,
    L2CAP_Echo_Response = 0x09,
    L2CAP_Information_Request = 0x0A,
    L2CAP_Information_Response = 0x0B
} L2CAP_SIGNALLING_COMMAND_CODE;

typedef enum _L2CAP_CONFIGURATION_RESPONSE_RESULT
{
    /// <summary>
    ///     Success
    /// </summary>
    L2CAP_ConfigurationResponseResult_Success = 0x0000,

    /// <summary>
    ///     Failure – unacceptable parameters
    /// </summary>
    L2CAP_ConfigurationResponseResult_FailureUnacceptableParameters = 0x0001,

    /// <summary>
    ///     Failure – rejected (no reason provided)
    /// </summary>
    L2CAP_ConfigurationResponseResult_FailureRejected = 0x0002,

    /// <summary>
    ///     Failure – unknown options
    /// </summary>
    L2CAP_ConfigurationResponseResult_FailureUnknownOptions = 0x0003
} L2CAP_CONFIGURATION_RESPONSE_RESULT;

typedef enum _L2CAP_CONNECTION_RESPONSE_RESULT
{
    /// <summary>
    ///     Connection successful.
    /// </summary>
    L2CAP_ConnectionResponseResult_ConnectionSuccessful = 0x0000,

    /// <summary>
    ///     Connection pending.
    /// </summary>
    L2CAP_ConnectionResponseResult_ConnectionPending = 0x0001,

    /// <summary>
    ///     Connection refused – PSM not supported.
    /// </summary>
    L2CAP_ConnectionResponseResult_ConnectionRefusedPsmNotNupported = 0x0002,

    /// <summary>
    ///     Connection refused – security block.
    /// </summary>
    L2CAP_ConnectionResponseResult_ConnectionRefusedSecurityBlock = 0x0003,

    /// <summary>
    ///     Connection refused – no resources available.
    /// </summary>
    L2CAP_ConnectionResponseResult_ConnectionRefusedNoResourcesAvailable = 0x0004
} L2CAP_CONNECTION_RESPONSE_RESULT;

typedef enum _L2CAP_CONNECTION_RESPONSE_STATUS
{
    /// <summary>
    ///     No further information available.
    /// </summary>
    L2CAP_ConnectionResponseStatus_NoFurtherInformationAvailable = 0x0000,
    /// <summary>
    ///     Authentication pending.
    /// </summary>
    L2CAP_ConnectionResponseStatus_AuthenticationPending = 0x0001,
    /// <summary>
    ///     Authorisation pending.
    /// </summary>
    L2CAP_ConnectionResponseStatus_AuthorisationPending = 0x0002
} L2CAP_CONNECTION_RESPONSE_STATUS;

typedef enum _L2CAP_PSM
{
    L2CAP_PSM_HID_Service = 0x01,
    L2CAP_PSM_HID_Command = 0x11,
    L2CAP_PSM_HID_Interrupt = 0x13
} L2CAP_PSM;

typedef struct _L2CAP_CID
{
    BYTE Lsb;
    BYTE Msb;
} L2CAP_CID;

#define L2CAP_IS_CONTROL_CHANNEL(_buf_)                     ((BOOLEAN)_buf_[6] == 0x01 && _buf_[7] == 0x00)
#define L2CAP_IS_HID_INPUT_REPORT(_buf_)                    ((BOOLEAN)_buf_[8] == 0xA1 && _buf_[9] == 0x01)
#define L2CAP_GET_SIGNALLING_COMMAND_CODE(_buf_)            ((L2CAP_SIGNALLING_COMMAND_CODE)_buf_[8])
#define L2CAP_GET_PROTOCOL_SERVICE_MULTIPLEXER(_buf_)       ((L2CAP_PSM)_buf_[12])
#define L2CAP_GET_CHANNEL_ID(_buf_)                         ((UCHAR)_buf_[9])

BOOLEAN FORCEINLINE L2CAP_IS_SIGNALLING_COMMAND_CODE(
    PUCHAR Buffer
)
{
    for (UCHAR i = L2CAP_Command_Reject; i <= L2CAP_Information_Response; i++)
    {
        if (i == Buffer[8]) return TRUE;
    }

    return FALSE;
}


NTSTATUS
L2CAP_Command(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    PVOID Buffer,
    ULONG BufferLength);

NTSTATUS
L2CAP_Command_Connection_Request(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID DestinationChannelId,
    L2CAP_PSM ProtocolServiceMultiplexer);

NTSTATUS
L2CAP_Command_Connection_Response(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID DestinationChannelId,
    L2CAP_CID SourceChannelId,
    L2CAP_CONNECTION_RESPONSE_RESULT Result,
    L2CAP_CONNECTION_RESPONSE_STATUS Status);

NTSTATUS
L2CAP_Command_Configuration_Request(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID DestinationChannelId,
    BOOLEAN SetMtu);

NTSTATUS
L2CAP_Command_Configuration_Response(
    PDEVICE_CONTEXT Context,
    BTH_HANDLE Handle,
    BYTE Id,
    L2CAP_CID SourceChannelId);
