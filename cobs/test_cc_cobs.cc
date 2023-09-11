#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cobs/cc_cobs.h"

using namespace testing;
using namespace cobs;

using Vectors = std::vector<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>>;

class TestVectorFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    vectors_.push_back({{}, {0x01, 0x00}});
    vectors_.push_back({{0x00}, {0x01, 0x01, 0x00}});
    vectors_.push_back({{0x00, 0x00}, {0x01, 0x01, 0x01, 0x00}});
    vectors_.push_back({{0x11, 0x22, 0x00, 0x33}, {0x03, 0x11, 0x22, 0x02, 0x33, 0x00}});
    vectors_.push_back({{0x11, 0x22, 0x33, 0x44}, {0x05, 0x11, 0x22, 0x33, 0x44, 0x00}});

    std::vector<uint8_t> decoded;
    std::vector<uint8_t> encoded;

    decoded = GetVector(254, 1);
    encoded = GetVector(256, 0);
    encoded[0] = 0xFF;
    encoded[255] = 0x00;
    vectors_.push_back({decoded, encoded});

    decoded = GetVector(255, 0);
    encoded = GetVector(257, static_cast<uint8_t>(-1));
    encoded[0] = 0x01;
    encoded[1] = 0xFF;
    encoded[256] = 0x00;
    vectors_.push_back({decoded, encoded});

    decoded = GetVector(255, 1);
    encoded = GetVector(258, 0);
    encoded[0] = 0xFF;
    encoded[255] = 0x02;
    encoded[256] = 0xFF;
    encoded[257] = 0x00;
    vectors_.push_back({decoded, encoded});

    decoded = GetVector(255, 2);
    encoded = GetVector(258, 1);
    encoded[0] = 0xFF;
    encoded[255] = 0x01;
    encoded[256] = 0x01;
    encoded[257] = 0x00;
    vectors_.push_back({decoded, encoded});

    decoded = GetVector(255, 3);
    encoded = GetVector(257, 2);
    encoded[0] = 0xFE;
    encoded[254] = 0x02;
    encoded[255] = 0x01;
    encoded[256] = 0x00;
    vectors_.push_back({decoded, encoded});
  }

  std::vector<uint8_t> GetVector(size_t size, uint8_t offset) {
    std::vector<uint8_t> output;
    for (size_t i = 0; i < size; ++i) {
      output.push_back(i + offset);
    }
    return output;
  }

  Vectors vectors_;
};

TEST_F(TestVectorFixture, EncodeSuccessNoAllocate) {
  for (size_t i = 0; i < vectors_.size(); ++i) {
    SCOPED_TRACE("Vector: " + std::to_string(i));
    auto& [decoded, encoded] = vectors_[i];

    std::vector<uint8_t> actual(encoded.size());
    const size_t actual_len = Encode(actual.data(), decoded.data(), decoded.size());

    EXPECT_EQ(actual_len, encoded.size());
    EXPECT_THAT(actual, ElementsAreArray(encoded));
  }
}

TEST_F(TestVectorFixture, EncodeSuccessAllocate) {
  for (size_t i = 0; i < vectors_.size(); ++i) {
    SCOPED_TRACE("Vector: " + std::to_string(i));
    auto& [decoded, encoded] = vectors_[i];

    std::vector<uint8_t> actual = Encode(decoded.data(), decoded.size());

    EXPECT_EQ(actual.size(), encoded.size());
    EXPECT_THAT(actual, ElementsAreArray(encoded));
  }
}

static void TestEncoderSuccess(Encoder& encoder, const Vectors& vectors) {
  for (size_t i = 0; i < vectors.size(); ++i) {
    SCOPED_TRACE("Vector: " + std::to_string(i));
    auto& [decoded, encoded] = vectors[i];

    encoder.Reset();

    constexpr size_t kBlockSize = 25;
    size_t j = 0;
    while (j < decoded.size()) {
      const size_t len = std::min(kBlockSize, decoded.size() - j);
      encoder.Encode(&decoded[j], len);
      j += len;
    }

    auto [actual_ptr, actual_len] = encoder.Get();
    std::vector<uint8_t> actual{actual_ptr, actual_ptr + actual_len};

    EXPECT_EQ(actual.size(), encoded.size());
    EXPECT_THAT(actual, ElementsAreArray(encoded));
  }
}

static void TestEncoderSuccessCopy(Encoder& encoder, const Vectors& vectors) {
  for (size_t i = 0; i < vectors.size(); ++i) {
    SCOPED_TRACE("Vector: " + std::to_string(i));
    auto& [decoded, encoded] = vectors[i];

    encoder.Reset();

    constexpr size_t kBlockSize = 25;
    size_t j = 0;
    while (j < decoded.size()) {
      const size_t len = std::min(kBlockSize, decoded.size() - j);
      encoder.Encode(&decoded[j], len);
      j += len;
    }

    std::vector<uint8_t> actual = encoder.GetCopy();

    EXPECT_EQ(actual.size(), encoded.size());
    EXPECT_THAT(actual, ElementsAreArray(encoded));
  }
}

TEST_F(TestVectorFixture, EncoderSuccess) {
  {
    std::vector<uint8_t> buf(1024);
    Encoder encoder(buf.data());

    TestEncoderSuccess(encoder, vectors_);
    TestEncoderSuccessCopy(encoder, vectors_);
  }
  {
    Encoder encoder(1024);
    TestEncoderSuccess(encoder, vectors_);
    TestEncoderSuccessCopy(encoder, vectors_);
  }
}

static void TestDecoderSuccess(Decoder& decoder, const Vectors& vectors) {
  for (size_t i = 0; i < vectors.size(); ++i) {
    SCOPED_TRACE("Vector: " + std::to_string(i));
    auto& [decoded, encoded] = vectors[i];

    decoder.Reset();

    for (size_t i = 0; i + 1 < encoded.size(); ++i) {
      auto [status, actual_span] = decoder.Decode(encoded[i]);
      EXPECT_EQ(Status::Processing, status);
      EXPECT_EQ(nullptr, actual_span.first);
      EXPECT_EQ(0, actual_span.second);
    }

    auto [status, actual_span] = decoder.Decode(encoded.back());

    ASSERT_EQ(status, Status::FrameAvailable);
    std::vector<uint8_t> actual{actual_span.first, actual_span.first + actual_span.second};
    EXPECT_THAT(actual, ElementsAreArray(decoded));
  }
}

static void TestDecoderSuccessCopy(Decoder& decoder, const Vectors& vectors) {
  for (size_t i = 0; i < vectors.size(); ++i) {
    SCOPED_TRACE("Vector: " + std::to_string(i));
    auto& [decoded, encoded] = vectors[i];

    decoder.Reset();

    for (size_t i = 0; i + 1 < encoded.size(); ++i) {
      auto [status, actual_span] = decoder.Decode(encoded[i]);
      EXPECT_EQ(Status::Processing, status);
      EXPECT_EQ(nullptr, actual_span.first);
      EXPECT_EQ(0, actual_span.second);
    }

    auto [status, actual] = decoder.DecodeAndCopy(encoded.back());

    EXPECT_EQ(status, Status::FrameAvailable);
    EXPECT_THAT(actual, ElementsAreArray(decoded));
  }
}

TEST_F(TestVectorFixture, DecoderSuccess) {
  {
    std::vector<uint8_t> buf(1024);
    Decoder decoder(buf.data(), buf.size());

    TestDecoderSuccess(decoder, vectors_);
    TestDecoderSuccessCopy(decoder, vectors_);
  }
  {
    Decoder decoder(1024);

    TestDecoderSuccess(decoder, vectors_);
    TestDecoderSuccessCopy(decoder, vectors_);
  }
}

static void TestDecoderOverflow(Decoder& decoder, size_t len) {
  {
    for (size_t i = 0; i < len + 1; ++i) {
      auto [status, actual_span] = decoder.Decode(0xFF);
      EXPECT_EQ(Status::Processing, status);
      EXPECT_EQ(nullptr, actual_span.first);
      EXPECT_EQ(0, actual_span.second);
    }

    auto [status, actual_span] = decoder.Decode(0xFF);
    EXPECT_EQ(Status::Overflow, status);
    EXPECT_EQ(nullptr, actual_span.first);
    EXPECT_EQ(0, actual_span.second);
  }

  decoder.Reset();

  {
    for (size_t i = 0; i < len + 1; ++i) {
      auto [status, actual] = decoder.DecodeAndCopy(0xFF);
      EXPECT_EQ(Status::Processing, status);
      EXPECT_EQ(0, actual.size());
    }

    auto [status, actual] = decoder.DecodeAndCopy(0xFF);
    EXPECT_EQ(Status::Overflow, status);
    EXPECT_EQ(0, actual.size());
  }
}

TEST(Decoder, Overflow) {
  constexpr size_t kLen = 3;
  {
    std::vector<uint8_t> buf(kLen);
    Decoder decoder(buf.data(), buf.size());
    TestDecoderOverflow(decoder, kLen);
  }
  {
    Decoder decoder(kLen);
    TestDecoderOverflow(decoder, kLen);
  }
}

static void TestDecoderMalformedFrame(Decoder& decoder) {
  const std::vector<uint8_t> encoded = {0xAA, 0xFF, 0xFF, 0xFF, 0x00};

  {
    for (size_t i = 0; i + 1 < encoded.size(); ++i) {
      auto [status, actual_span] = decoder.Decode(encoded[i]);
      EXPECT_EQ(Status::Processing, status);
      EXPECT_EQ(nullptr, actual_span.first);
      EXPECT_EQ(0, actual_span.second);
    }

    auto [status, actual_span] = decoder.Decode(encoded.back());
    EXPECT_EQ(Status::MalformedFrame, status);
    EXPECT_EQ(nullptr, actual_span.first);
    EXPECT_EQ(0, actual_span.second);
  }

  decoder.Reset();

  {
    for (size_t i = 0; i + 1 < encoded.size(); ++i) {
      auto [status, actual] = decoder.DecodeAndCopy(encoded[i]);
      EXPECT_EQ(Status::Processing, status);
      EXPECT_EQ(0, actual.size());
    }

    auto [status, actual] = decoder.DecodeAndCopy(encoded.back());
    EXPECT_EQ(Status::MalformedFrame, status);
    EXPECT_EQ(0, actual.size());
  }
}

TEST(Decoder, MalformedFrame) {
  {
    std::vector<uint8_t> buf(1024);
    Decoder decoder(buf.data(), buf.size());
    TestDecoderMalformedFrame(decoder);
  }
  {
    Decoder decoder(1024);
    TestDecoderMalformedFrame(decoder);
  }
}

TEST_F(TestVectorFixture, DecodeSuccessNoAllocate) {
  for (size_t i = 0; i < vectors_.size(); ++i) {
    SCOPED_TRACE("Vector: " + std::to_string(i));
    auto& [decoded, encoded] = vectors_[i];

    std::vector<uint8_t> actual(decoded.size());
    size_t actual_len = actual.size();
    Status status = Decode(actual.data(), &actual_len, encoded.data(), encoded.size());

    EXPECT_EQ(status, Status::FrameAvailable);
    EXPECT_EQ(actual_len, decoded.size());
    EXPECT_THAT(actual, ElementsAreArray(decoded));
  }
}

TEST_F(TestVectorFixture, DecodeSuccessAllocate) {
  for (size_t i = 0; i < vectors_.size(); ++i) {
    SCOPED_TRACE("Vector: " + std::to_string(i));
    auto& [decoded, encoded] = vectors_[i];

    auto [status, actual] = Decode(encoded.data(), encoded.size());

    EXPECT_EQ(status, Status::FrameAvailable);
    EXPECT_THAT(actual, ElementsAreArray(decoded));
  }
}

TEST(Decode, IncompleteFrame) {
  std::vector<uint8_t> encoded = {0x01, 0x01};

  {
    // This is the same size as the encoded buffer as opposed to MaxDecodeLen(encoded.size()) so
    // that we get an incomplete frame vs. overflow.
    std::vector<uint8_t> actual(encoded.size());
    size_t actual_len = actual.size();
    Status status = Decode(actual.data(), &actual_len, encoded.data(), encoded.size());

    EXPECT_EQ(status, Status::IncompleteFrame);
    EXPECT_EQ(actual_len, 0);
  }

  {
    auto [status, actual] = Decode(encoded.data(), encoded.size());

    EXPECT_EQ(status, Status::IncompleteFrame);
    EXPECT_EQ(actual.size(), 0);
  }
}

TEST(MaxLengths, Encode) {
  EXPECT_EQ(2, MaxEncodeLen(0));
  EXPECT_EQ(3, MaxEncodeLen(1));
  EXPECT_EQ(256, MaxEncodeLen(254));
  EXPECT_EQ(258, MaxEncodeLen(255));
  EXPECT_EQ(511, MaxEncodeLen(508));
  EXPECT_EQ(513, MaxEncodeLen(509));
}

TEST(MaxLengths, Decode) {
  EXPECT_EQ(0, MaxDecodeLen(0));
  EXPECT_EQ(0, MaxDecodeLen(1));
  EXPECT_EQ(0, MaxDecodeLen(2));
  EXPECT_EQ(1, MaxDecodeLen(3));
  EXPECT_EQ(1000, MaxDecodeLen(1002));
}
