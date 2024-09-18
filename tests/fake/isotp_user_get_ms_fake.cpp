
#include <CppUTestExt/MockSupport.h>
#include <isotp.h>

extern "C" {

static uint32_t _fake_ms = 0;

uint32_t isotp_user_get_ms(void) {
    return _fake_ms;
}

}  // extern "C"

void fake_forward_time_ms(uint32_t ms) {
    _fake_ms += ms;
}

void fake_reset_time(void) {
    _fake_ms = 0;
}
