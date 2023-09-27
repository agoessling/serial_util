#include <stdint.h>

#include "external/unity/src/unity.h"

#include "crc/c_crc.h"
#include "crc/all_crcs.h"

static const uint8_t g_check[] = "123456789";

void setUp(void) {}
void tearDown(void) {}

static void TestCrc8Darc(void) {
  TEST_ASSERT_EQUAL_HEX8(0x15, Crc8Block(&kCrc8DarcInfo, g_check, sizeof(g_check) - 1));
}

static void TestCrc8ICode(void) {
  TEST_ASSERT_EQUAL_HEX8(0x7E, Crc8Block(&kCrc8ICodeInfo, g_check, sizeof(g_check) - 1));
}

static void TestCrc16Kermit(void) {
  TEST_ASSERT_EQUAL_HEX16(0x2189, Crc16Block(&kCrc16KermitInfo, g_check, sizeof(g_check) - 1));
}

static void TestCrc16CcittFalse(void) {
  TEST_ASSERT_EQUAL_HEX16(0x29B1, Crc16Block(&kCrc16CcittFalseInfo, g_check, sizeof(g_check) - 1));
}

static void TestCrc32(void) {
  TEST_ASSERT_EQUAL_HEX32(0xCBF43926, Crc32Block(&kCrc32Info, g_check, sizeof(g_check) - 1));
}

static void TestCrc32Mpeg2(void) {
  TEST_ASSERT_EQUAL_HEX32(0x0376E6E7, Crc32Block(&kCrc32Mpeg2Info, g_check, sizeof(g_check) - 1));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(TestCrc8Darc);
  RUN_TEST(TestCrc8ICode);
  RUN_TEST(TestCrc16Kermit);
  RUN_TEST(TestCrc16CcittFalse);
  RUN_TEST(TestCrc32);
  RUN_TEST(TestCrc32Mpeg2);
  return UNITY_END();
}
