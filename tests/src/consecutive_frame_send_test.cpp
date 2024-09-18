#include "CppUTest/TestHarness.h"
#include <CppUTestExt/MockSupport.h>

#include <fake/isotp_user_get_ms_fake.hpp>
#include <mock/isotp_user_send_can_mock.hpp>

#include "../isotp.h"

TEST_GROUP(consecutive_frame_send_test) {

    isotp_link_t link;
    static const size_t RX_TX_MAX_SIZE = 32;
    uint8_t tx_buf[RX_TX_MAX_SIZE];
    uint8_t rx_buf[RX_TX_MAX_SIZE];

    void setup() final {

        fake_reset_time();

        isotp_init_link(&link, 0x123, tx_buf, RX_TX_MAX_SIZE, rx_buf, RX_TX_MAX_SIZE);

        mock().enable();
        mock().strictOrder();
        mock().clear();
    }

    void teardown() final {
        mock().checkExpectations();
        mock().clear();
        mock().disable();
    }
};

TEST(consecutive_frame_send_test, consecutive_frame_send_8byte_2_frames_no_flow_control) {

    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };
    const uint8_t EXPECTED_DATA_1[] = { 0x10, 0x08, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
    const uint8_t EXPECTED_DATA_2[] = { 0x21, 0xDE, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00 };

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_1, sizeof(EXPECTED_DATA_1), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);

    isotp_poll(&link);

    const uint8_t SEPARATION_TIME_MS = 0;
    const uint8_t FLOW_CONTROL_FRAMES_COUNT = 0;
    const uint8_t FLOW_CONTROL_FRAME[] = { 0x30, FLOW_CONTROL_FRAMES_COUNT, SEPARATION_TIME_MS, 0xff, 0xff, 0xff, 0xff,
                                           0xff };
    isotp_on_can_message(&link, FLOW_CONTROL_FRAME, sizeof(FLOW_CONTROL_FRAME));

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_2, sizeof(EXPECTED_DATA_2), ISOTP_RET_OK);
    isotp_poll(&link);
}

TEST(consecutive_frame_send_test, consecutive_frame_send_8byte_2_frames_flow_control) {

    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };
    const uint8_t EXPECTED_DATA_1[] = { 0x10, 0x08, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
    const uint8_t EXPECTED_DATA_2[] = { 0x21, 0xDE, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00 };

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_1, sizeof(EXPECTED_DATA_1), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);

    isotp_poll(&link);

    const uint8_t SEPARATION_TIME_MS = 10;
    const uint8_t FLOW_CONTROL_FRAMES_COUNT = 1;
    const uint8_t FLOW_CONTROL_FRAME[] = { 0x30, FLOW_CONTROL_FRAMES_COUNT, SEPARATION_TIME_MS, 0xff, 0xff, 0xff, 0xff,
                                           0xff };
    isotp_on_can_message(&link, FLOW_CONTROL_FRAME, sizeof(FLOW_CONTROL_FRAME));

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_2, sizeof(EXPECTED_DATA_2), ISOTP_RET_OK);
    fake_forward_time_ms(1);  // TODO fix it. send immediately
    isotp_poll(&link);
}

TEST(consecutive_frame_send_test, consecutive_frame_send_8byte_5_frames_no_flow_control) {

    uint8_t DATA[RX_TX_MAX_SIZE];
    for (size_t i = 0; i < RX_TX_MAX_SIZE; i++) {
        DATA[i] = i;
    }

    const uint8_t EXPECTED_DATA_1[] = { 0x10, 0x20, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
    const uint8_t EXPECTED_DATA_2[] = { 0x21, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c };
    const uint8_t EXPECTED_DATA_3[] = { 0x22, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13 };
    const uint8_t EXPECTED_DATA_4[] = { 0x23, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a };
    const uint8_t EXPECTED_DATA_5[] = { 0x24, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x00, 0x00 };

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_1, sizeof(EXPECTED_DATA_1), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);

    isotp_poll(&link);

    const uint8_t SEPARATION_TIME_MS = 0;
    const uint8_t FLOW_CONTROL_FRAMES_COUNT = 0;
    const uint8_t FLOW_CONTROL_FRAME[] = { 0x30, FLOW_CONTROL_FRAMES_COUNT, SEPARATION_TIME_MS, 0xff, 0xff, 0xff, 0xff,
                                           0xff };
    isotp_on_can_message(&link, FLOW_CONTROL_FRAME, sizeof(FLOW_CONTROL_FRAME));

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_2, sizeof(EXPECTED_DATA_2), ISOTP_RET_OK);
    isotp_poll(&link);

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_3, sizeof(EXPECTED_DATA_3), ISOTP_RET_OK);
    isotp_poll(&link);

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_4, sizeof(EXPECTED_DATA_4), ISOTP_RET_OK);
    isotp_poll(&link);

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_5, sizeof(EXPECTED_DATA_5), ISOTP_RET_OK);
    isotp_poll(&link);
}

TEST(consecutive_frame_send_test, consecutive_frame_send_8byte_5_frames_flow_control) {

    uint8_t DATA[RX_TX_MAX_SIZE];
    for (size_t i = 0; i < RX_TX_MAX_SIZE; i++) {
        DATA[i] = i;
    }

    const uint8_t EXPECTED_DATA_1[] = { 0x10, 0x20, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
    const uint8_t EXPECTED_DATA_2[] = { 0x21, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c };
    const uint8_t EXPECTED_DATA_3[] = { 0x22, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13 };
    const uint8_t EXPECTED_DATA_4[] = { 0x23, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a };
    const uint8_t EXPECTED_DATA_5[] = { 0x24, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x00, 0x00 };

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_1, sizeof(EXPECTED_DATA_1), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);

    isotp_poll(&link);

    const uint8_t SEPARATION_TIME_MS = 10;
    const uint8_t FLOW_CONTROL_FRAMES_COUNT = 5;
    const uint8_t FLOW_CONTROL_FRAME[] = { 0x30, FLOW_CONTROL_FRAMES_COUNT, SEPARATION_TIME_MS, 0xff, 0xff, 0xff, 0xff,
                                           0xff };
    isotp_on_can_message(&link, FLOW_CONTROL_FRAME, sizeof(FLOW_CONTROL_FRAME));

    expect_isotp_user_send_can(0x123, EXPECTED_DATA_2, sizeof(EXPECTED_DATA_2), ISOTP_RET_OK);
    fake_forward_time_ms(1);  // TODO fix it. send immediately
    isotp_poll(&link);

    /* Forward to edge and chen non-expected and then forward to one */
    fake_forward_time_ms(SEPARATION_TIME_MS);
    isotp_poll(&link);
    mock().checkExpectations();
    fake_forward_time_ms(1);
    expect_isotp_user_send_can(0x123, EXPECTED_DATA_3, sizeof(EXPECTED_DATA_3), ISOTP_RET_OK);
    isotp_poll(&link);

    /* Forward to edge and chen non-expected and then forward to one */
    fake_forward_time_ms(SEPARATION_TIME_MS);
    isotp_poll(&link);
    mock().checkExpectations();
    fake_forward_time_ms(1);
    expect_isotp_user_send_can(0x123, EXPECTED_DATA_4, sizeof(EXPECTED_DATA_4), ISOTP_RET_OK);
    isotp_poll(&link);

    /* Forward to edge and chen non-expected and then forward to one */
    fake_forward_time_ms(SEPARATION_TIME_MS);
    isotp_poll(&link);
    mock().checkExpectations();
    fake_forward_time_ms(1);
    expect_isotp_user_send_can(0x123, EXPECTED_DATA_5, sizeof(EXPECTED_DATA_5), ISOTP_RET_OK);
    isotp_poll(&link);
}
