#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* user implemented, send can message. should return ISOTP_RET_OK when success. */
int isotp_user_send_can(const uint8_t can_iface_id,
                        const uint32_t arbitration_id,
                        const uint8_t *data,
                        const uint8_t size);

/* user implemented, get millisecond */
uint32_t isotp_user_get_ms(void);

#ifdef __cplusplus
}
#endif
