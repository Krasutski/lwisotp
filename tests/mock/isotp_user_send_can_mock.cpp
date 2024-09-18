
#include <CppUTestExt/MockSupport.h>
#include <isotp.h>

extern "C" {

#if defined(DEBUG)
void print_array(const uint8_t *data, const uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\n");
}
#endif

int isotp_user_send_can(const uint8_t can_iface_id,
                        const uint32_t arbitration_id,
                        const uint8_t *data,
                        const uint8_t size) {
#if defined(DEBUG)
    print_array(data, size);
#endif
    return mock()
        .actualCall("isotp_user_send_can")
        .withUnsignedIntParameter("can_iface_id", can_iface_id)
        .withUnsignedIntParameter("arbitration_id", arbitration_id)
        .withMemoryBufferParameter("data", data, size)
        .withUnsignedIntParameter("size", size)
        .returnIntValue();
}
}

void expect_isotp_user_send_can(const uint8_t can_iface_id,
                                const uint32_t arbitration_id,
                                const uint8_t *data,
                                const uint8_t size,
                                const int return_value) {
    mock()
        .expectOneCall("isotp_user_send_can")
        .withUnsignedIntParameter("can_iface_id", can_iface_id)
        .withUnsignedIntParameter("arbitration_id", arbitration_id)
        .withMemoryBufferParameter("data", data, size)
        .withUnsignedIntParameter("size", size)
        .andReturnValue(return_value);
}
