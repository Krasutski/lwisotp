#pragma once

#include <stdint.h>

void expect_isotp_user_send_can(void *context,
                                const uint32_t arbitration_id,
                                const uint8_t *data,
                                const uint8_t size,
                                const int return_value);
