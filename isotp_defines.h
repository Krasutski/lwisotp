#pragma once

/**************************************************************
 * compiler specific defines
 *************************************************************/
#ifdef __GNUC__
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#        define ISOTP_BYTE_ORDER_LITTLE_ENDIAN
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#    else
#        error "unsupported byte ordering"
#    endif
#elif defined(__CC_ARM)
#    define ISOTP_BYTE_ORDER_LITTLE_ENDIAN
#else
#endif

/**************************************************************
 * internal used defines
 *************************************************************/
typedef enum {
    ISOTP_RET_OK = 0,
    ISOTP_RET_ERROR = -1,
    ISOTP_RET_IN_PROGRESS = -2,
    ISOTP_RET_OVERFLOW = -3,
    ISOTP_RET_WRONG_SN = -4,
    ISOTP_RET_NO_DATA = -5,
    ISOTP_RET_TIMEOUT = -6,
    ISOTP_RET_LENGTH = -7,
} isotp_result_t;

/*  invalid bs */
#define ISOTP_INVALID_BS 0xFFFF

/* Minimum flow control frame size */
#define ISOTP_MIN_FLOW_CONTROL_FRAME_SIZE 3

/* ISOTP sender status */
typedef enum {
    ISOTP_SEND_STATUS_IDLE,
    ISOTP_SEND_STATUS_IN_PROGRESS,
    ISOTP_SEND_STATUS_ERROR,
} isotp_send_status_type_t;

/* ISOTP receiver status */
typedef enum {
    ISOTP_RECEIVE_STATUS_IDLE,
    ISOTP_RECEIVE_STATUS_IN_PROGRESS,
    ISOTP_RECEIVE_STATUS_FULL,
} isotp_receive_status_type_t;

/* can frame defintion */
#if defined(ISOTP_BYTE_ORDER_LITTLE_ENDIAN)
typedef struct {
    uint8_t reserve_1 : 4;
    uint8_t type      : 4;
    uint8_t reserve_2[7];
} isotp_pci_type_t;

typedef struct {
    uint8_t sf_dl : 4;
    uint8_t type  : 4;
    uint8_t data[7];
} isotp_single_frame_t;

typedef struct {
    uint8_t ff_dl_high : 4;
    uint8_t type       : 4;
    uint8_t ff_dl_low;
    uint8_t data[6];
} isotp_first_frame_t;

typedef struct {
    uint8_t sn   : 4;
    uint8_t type : 4;
    uint8_t data[7];
} isotp_consecutive_frame_t;

typedef struct {
    uint8_t fs   : 4;
    uint8_t type : 4;
    uint8_t bs;
    uint8_t st_min;
    uint8_t reserve[5];
} isotp_flow_control_t;

#else

typedef struct {
    uint8_t type      : 4;
    uint8_t reserve_1 : 4;
    uint8_t reserve_2[7];
} isotp_pci_type_t;

/*
 * single frame
 * +-------------------------+-----+
 * | byte #0                 | ... |
 * +-------------------------+-----+
 * | nibble #0   | nibble #1 | ... |
 * +-------------+-----------+ ... +
 * | PCIType = 0 | SF_DL     | ... |
 * +-------------+-----------+-----+
 */
typedef struct {
    uint8_t type  : 4;
    uint8_t sf_dl : 4;
    uint8_t data[7];
} isotp_single_frame_t;

/*
 * first frame
 * +-------------------------+-----------------------+-----+
 * | byte #0                 | byte #1               | ... |
 * +-------------------------+-----------+-----------+-----+
 * | nibble #0   | nibble #1 | nibble #2 | nibble #3 | ... |
 * +-------------+-----------+-----------+-----------+-----+
 * | PCIType = 1 | FF_DL                             | ... |
 * +-------------+-----------+-----------------------+-----+
 */
typedef struct {
    uint8_t type       : 4;
    uint8_t ff_dl_high : 4;
    uint8_t ff_dl_low;
    uint8_t data[6];
} isotp_first_frame_t;

/*
 * consecutive frame
 * +-------------------------+-----+
 * | byte #0                 | ... |
 * +-------------------------+-----+
 * | nibble #0   | nibble #1 | ... |
 * +-------------+-----------+ ... +
 * | PCIType = 0 | SN        | ... |
 * +-------------+-----------+-----+
 */
typedef struct {
    uint8_t type : 4;
    uint8_t sn   : 4;
    uint8_t data[7];
} isotp_consecutive_frame_t;

/*
 * flow control frame
 * +-------------------------+-----------------------+-----------------------+-----+
 * | byte #0                 | byte #1               | byte #2               | ... |
 * +-------------------------+-----------+-----------+-----------+-----------+-----+
 * | nibble #0   | nibble #1 | nibble #2 | nibble #3 | nibble #4 | nibble #5 | ... |
 * +-------------+-----------+-----------+-----------+-----------+-----------+-----+
 * | PCIType = 1 | FS        | BS                    | STmin                 | ... |
 * +-------------+-----------+-----------------------+-----------------------+-----+
 */
typedef struct {
    uint8_t type : 4;
    uint8_t fs   : 4;
    uint8_t bs;
    uint8_t st_min;
    uint8_t reserve[5];
} isotp_flow_control_t;

#endif

typedef struct {
    uint8_t ptr[8];
} isotp_data_array_t;

typedef struct {
    union {
        isotp_pci_type_t common;
        isotp_single_frame_t single_frame;
        isotp_first_frame_t first_frame;
        isotp_consecutive_frame_t consecutive_frame;
        isotp_flow_control_t flow_control;
        isotp_data_array_t data_array;
    } as;
} isotp_can_message_t;

/**************************************************************
 * protocol specific defines
 *************************************************************/

/* Private: Protocol Control Information (PCI) types, for identifying each frame of an ISO-TP message. */
typedef enum {
    ISOTP_PCI_TYPE_SINGLE = 0x0,
    ISOTP_PCI_TYPE_FIRST_FRAME = 0x1,
    ISOTP_PCI_TYPE_CONSECUTIVE_FRAME = 0x2,
    ISOTP_PCI_TYPE_FLOW_CONTROL_FRAME = 0x3
} isotp_protocol_control_information_t;

/* Private: Protocol Control Information (PCI) flow control identifiers. */
typedef enum {
    PCI_FLOW_STATUS_CONTINUE = 0x0,
    PCI_FLOW_STATUS_WAIT = 0x1,
    PCI_FLOW_STATUS_OVERFLOW = 0x2
} isotp_flow_status_t;

/* Private: network layer result code. */
#define ISOTP_PROTOCOL_RESULT_OK              0
#define ISOTP_PROTOCOL_RESULT_TIMEOUT_A       -1
#define ISOTP_PROTOCOL_RESULT_TIMEOUT_BS      -2
#define ISOTP_PROTOCOL_RESULT_TIMEOUT_CR      -3
#define ISOTP_PROTOCOL_RESULT_WRONG_SN        -4
#define ISOTP_PROTOCOL_RESULT_INVALID_FS      -5
#define ISOTP_PROTOCOL_RESULT_UNEXPECTED_PDU  -6
#define ISOTP_PROTOCOL_RESULT_WFT_OVERRUN     -7
#define ISOTP_PROTOCOL_RESULT_BUFFER_OVERFLOW -8
#define ISOTP_PROTOCOL_RESULT_ERROR           -9
