#include "CppUTest/TestHarness.h"
#include <CppUTestExt/MockSupport.h>

#include <fake/isotp_user_get_ms_fake.hpp>
#include <mock/isotp_user_send_can_mock.hpp>

#include "../isotp.h"

TEST_GROUP(send_results_test) {

    isotp_link_t link;
    const uint8_t CAN_IFACE_ID = 0;
    const uint32_t CAN_MESSAGE_ID = 0x123456;
    static const size_t RX_TX_MAX_SIZE = 32;
    uint8_t tx_buf[RX_TX_MAX_SIZE];
    uint8_t rx_buf[RX_TX_MAX_SIZE];

    void setup() final {

        fake_reset_time();

        isotp_init_link(&link, CAN_IFACE_ID, CAN_MESSAGE_ID, tx_buf, RX_TX_MAX_SIZE, rx_buf, RX_TX_MAX_SIZE);

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

TEST(send_results_test, link_zero) {

    const uint8_t DATA[] = { 0x12 };
    CHECK_EQUAL(ISOTP_RET_ERROR, isotp_send(NULL, DATA, sizeof(DATA)));
}

TEST(send_results_test, isotp_send_with_id_link_zero) {

    const uint8_t DATA[] = { 0x12 };
    CHECK_EQUAL(ISOTP_RET_ERROR, isotp_send_with_id(NULL, CAN_MESSAGE_ID, DATA, sizeof(DATA)));
}

TEST(send_results_test, large_messsage) {

    const uint8_t DATA[] = { 0x12 };
    CHECK_EQUAL(ISOTP_RET_OVERFLOW, isotp_send(&link, DATA, RX_TX_MAX_SIZE + 1));
}

TEST(send_results_test, send_error_low_level_single) {

    /* Send two frame message */
    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE };
    const uint8_t EXPECTED_DATA[] = { 0x07, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE };
    expect_isotp_user_send_can(CAN_IFACE_ID, CAN_MESSAGE_ID, EXPECTED_DATA, sizeof(EXPECTED_DATA), -12345);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(-12345, result);
}

TEST(send_results_test, send_error_low_level_multi) {

    /* Send two frame message */
    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };
    const uint8_t EXPECTED_DATA[] = { 0x10, 0x08, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
    expect_isotp_user_send_can(CAN_IFACE_ID, CAN_MESSAGE_ID, EXPECTED_DATA, sizeof(EXPECTED_DATA), -12345);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(-12345, result);
}

TEST(send_results_test, in_progress) {

    /* Send two frame message */
    const uint8_t DATA[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };
    const uint8_t EXPECTED_DATA[] = { 0x10, 0x08, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
    expect_isotp_user_send_can(CAN_IFACE_ID, CAN_MESSAGE_ID, EXPECTED_DATA, sizeof(EXPECTED_DATA), ISOTP_RET_OK);
    int result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_OK, result);

    /* Repeat send, sencond frame still not send, error  should be return */
    result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_IN_PROGRESS, result);

    isotp_poll(&link);

    /* Repeat send, sencond frame still not send, error  should be return */
    result = isotp_send(&link, DATA, sizeof(DATA));
    CHECK_EQUAL(ISOTP_RET_IN_PROGRESS, result);
}
