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

typedef enum _L2CAP_PSM
{
    L2CAP_PSM_HID_Service = 0x01,
    L2CAP_PSM_HID_Command = 0x11,
    L2CAP_PSM_HID_Interrupt = 0x13
} L2CAP_PSM;

#define L2CAP_IS_CONTROL_CHANNEL(_buf_)                     ((BOOLEAN)_buf_[6] == 0x01 && _buf_[7] == 0x00)
#define L2CAP_IS_HID_INPUT_REPORT(_buf_)                    ((BOOLEAN)_buf_[8] == 0xA1 && _buf_[9] == 0x01)
#define L2CAP_GET_SIGNALLING_COMMAND_CODE(_buf_)            ((L2CAP_SIGNALLING_COMMAND_CODE)_buf_[8])
#define L2CAP_GET_PROTOCOL_SERVICE_MULTIPLEXER(_buf_)       ((L2CAP_PSM)_buf_[12])

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
