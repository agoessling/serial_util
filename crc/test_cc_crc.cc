#include <cstdint>
#include <variant>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "crc/cc_crc.h"

extern "C" {
#include "crc/all_crcs.h"
}

using namespace testing;
using namespace crc;

struct CrcFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    crcs_.push_back({Crc<8>(&kCrc8DarcInfo), 0x15});
    crcs_.push_back({Crc<8>(&kCrc8ICodeInfo), 0x7E});
    crcs_.push_back({Crc<16>(&kCrc16KermitInfo), 0x2189});
    crcs_.push_back({Crc<16>(&kCrc16CcittFalseInfo), 0x29B1});
    crcs_.push_back({Crc<32>(&kCrc32Info), 0xCBF43926});
    crcs_.push_back({Crc<32>(&kCrc32Mpeg2Info), 0x0376E6E7});
  }

  std::vector<std::pair<std::variant<Crc<8>, Crc<16>, Crc<32>>, uint32_t>> crcs_;
  const uint8_t test_msg_[10] = "123456789";
};

TEST_F(CrcFixture, Block) {
  int i = 0;
  for (auto& [crc, value] : crcs_) {
    SCOPED_TRACE("Crc: " + std::to_string(i++));

    uint32_t actual = std::visit(
        [this](auto&& c) {
          return static_cast<uint32_t>(c.Block(test_msg_, sizeof(test_msg_) - 1));
        },
        crc);
    EXPECT_EQ(actual, value);
  }
}

TEST_F(CrcFixture, ByByte) {
  int i = 0;
  for (auto& [crc, value] : crcs_) {
    SCOPED_TRACE("Crc: " + std::to_string(i++));

    for (int i = 0; i < 4; ++i) {
      std::visit([i](auto&& c) { c(i); }, crc);
    }

    std::visit([](auto&& c) { c.Reset(); }, crc);

    for (size_t i = 0; i < sizeof(test_msg_) - 1; ++i) {
      SCOPED_TRACE("Byte: " + std::to_string(i));
      uint32_t actual =
          std::visit([this, i](auto&& c) { return static_cast<uint32_t>(c(test_msg_[i])); }, crc);
      if (i < sizeof(test_msg_) - 2) {
        EXPECT_NE(actual, value);
      } else {
        EXPECT_EQ(actual, value);
      }
    }
  }
}

TEST_F(CrcFixture, BySeq) {
  int i = 0;
  for (auto& [crc, value] : crcs_) {
    SCOPED_TRACE("Crc: " + std::to_string(i++));

    for (int i = 0; i < 4; ++i) {
      std::visit([i](auto&& c) { c(i); }, crc);
    }

    std::visit([](auto&& c) { c.Reset(); }, crc);

    size_t first_len = (sizeof(test_msg_) - 1) / 2;
    uint32_t actual = std::visit(
        [this, first_len](auto&& c) { return static_cast<uint32_t>(c(test_msg_, first_len)); },
        crc);
    EXPECT_NE(actual, value);

    size_t sec_len = (sizeof(test_msg_) - 1) - first_len;
    actual = std::visit(
        [this, first_len, sec_len](auto&& c) {
          return static_cast<uint32_t>(c(&test_msg_[first_len], sec_len));
        },
        crc);
    EXPECT_EQ(actual, value);
  }
}
