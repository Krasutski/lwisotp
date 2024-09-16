#ifndef __ISOTP_CONFIG__
#define __ISOTP_CONFIG__

/* Max number of messages the receiver can receive at one time, this value 
 * is affectied by can driver queue length
 */
#define CONFIG_ISOTP_DEFAULT_BLOCK_SIZE         8

/* The STmin parameter value specifies the minimum time gap allowed between 
 * the transmission of consecutive frame network protocol data units
 */
#define CONFIG_ISOTP_DEFAULT_ST_MIN             0

/* This parameter indicate how many FC N_PDU WTs can be transmitted by the 
 * receiver in a row.
 */
#define CONFIG_ISOTP_MAX_WFT_NUMBER             1

/* Private: The default timeout to use when waiting for a response during a
 * multi-frame send or receive.
 */
#define CONFIG_ISOTP_DEFAULT_RESPONSE_TIMEOUT   250

/* Private: Determines if by default, padding is added to ISO-TP message frames.
 */
#define CONFIG_ISOTP_FRAME_PADDING              1


/* Enable or disable the assert function */
#define CONFIG_ISOTP_ENABLE_ASSERT              1

#endif

