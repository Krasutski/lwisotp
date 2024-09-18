#include "CppUTest/TestHarness.h"

TEST_GROUP(dummy_test) {
    void setup() final {
        //
    }

    void teardown() final {
        //
    }
};

TEST(dummy_test, pass_me) {
    CHECK_EQUAL(1, 1);
}
