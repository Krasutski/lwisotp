#include "CppUTest/TestHarness.h"
#include <CppUTestExt/MockSupport.h>

#include <fake/isotp_user_get_ms_fake.hpp>
#include <mock/isotp_user_send_can_mock.hpp>

#include "../isotp.h"

TEST_GROUP(single_frame_send) {

    isotp_link_t link;
    static const size_t RX_TX_MAX_SIZE = 8;
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

TEST(single_frame_send, send_1byte) {

    const uint8_t DATA[] = { 0x12 };
    const uint8_t EXPECTED_DATA[] = { 0x01, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    expect_isotp_user_send_can(0x123, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);
}

TEST(single_frame_send, send_2byte) {

    const uint8_t DATA[] = { 0x12, 0x34 };
    const uint8_t EXPECTED_DATA[] = { 0x02, 0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00 };
    expect_isotp_user_send_can(0x123, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);
}

TEST(single_frame_send, send_3byte) {

    const uint8_t DATA[] = { 0x12, 0x34, 0x56 };
    const uint8_t EXPECTED_DATA[] = { 0x03, 0x12, 0x34, 0x56, 0x00, 0x00, 0x00, 0x00 };
    expect_isotp_user_send_can(0x123, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);
}

TEST(single_frame_send, send_4byte) {

    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78 };
    const uint8_t EXPECTED_DATA[] = { 0x04, 0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00 };
    expect_isotp_user_send_can(0x123, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);
}

TEST(single_frame_send, send_5byte) {

    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A };
    const uint8_t EXPECTED_DATA[] = { 0x05, 0x12, 0x34, 0x56, 0x78, 0x9A, 0x00, 0x00 };
    expect_isotp_user_send_can(0x123, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);
}

TEST(single_frame_send, send_6byte) {

    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
    const uint8_t EXPECTED_DATA[] = { 0x06, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0x00 };
    expect_isotp_user_send_can(0x123, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);
}

TEST(single_frame_send, send_7byte) {

    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE };
    const uint8_t EXPECTED_DATA[] = { 0x07, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE };
    expect_isotp_user_send_can(0x123, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);
}

TEST(single_frame_send, non_signle_frame_send_8byte) {

    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };
    const uint8_t EXPECTED_DATA[] = { 0x10, 0x08, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
    expect_isotp_user_send_can(0x123, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);
}
