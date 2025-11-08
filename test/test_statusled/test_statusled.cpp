#include <unity.h>
#include "StatusLED.h"
#include "fakes/MockDMXStatus.h"
#include "fakes/ArduinoFake.h"

void test_statusled_blinks_when_idle(void) {
    MockDMXStatus mockStatus;
    mockStatus.receiving = false;

    StatusLED led(mockStatus, LED_BUILTIN, 100, false);
    led.begin();

    fakeMillis = 0;
    led.update();
    TEST_ASSERT_EQUAL(LOW, fakePinStates[LED_BUILTIN]);

    fakeMillis = 50;
    led.update();
    TEST_ASSERT_EQUAL(LOW, fakePinStates[LED_BUILTIN]);

    fakeMillis = 150;
    led.update();
    TEST_ASSERT_EQUAL(HIGH, fakePinStates[LED_BUILTIN]);

    fakeMillis = 300;
    led.update();
    TEST_ASSERT_EQUAL(LOW, fakePinStates[LED_BUILTIN]);
}

void test_statusled_solid_on_when_receiving_high_fps(void) {
    MockDMXStatus mockStatus;
    mockStatus.receiving = true;
    mockStatus.fps = 30;

    StatusLED led(mockStatus, LED_BUILTIN, 500, false);
    led.begin();

    fakeMillis = 0;
    led.update();
    TEST_ASSERT_EQUAL(HIGH, fakePinStates[LED_BUILTIN]);
}

void test_statusled_off_when_receiving_low_fps(void) {
    MockDMXStatus mockStatus;
    mockStatus.receiving = true;
    mockStatus.fps = 10;

    StatusLED led(mockStatus, LED_BUILTIN, 500, false);
    led.begin();

    fakeMillis = 0;
    led.update();
    TEST_ASSERT_EQUAL(LOW, fakePinStates[LED_BUILTIN]);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_statusled_blinks_when_idle);
    RUN_TEST(test_statusled_solid_on_when_receiving_high_fps);
    RUN_TEST(test_statusled_off_when_receiving_low_fps);
    UNITY_END();
    return 0;
}
