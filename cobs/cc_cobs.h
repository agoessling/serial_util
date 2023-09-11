#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace cobs {

namespace impl {
extern "C" {
#include "cobs/c_cobs.h"
}
}  // namespace impl

enum class Status {
  Processing = impl::kCobsStatusProcessing,
  FrameAvailable = impl::kCobsStatusFrameAvailable,
  MalformedFrame = impl::kCobsStatusMalformedFrame,
  Overflow = impl::kCobsStatusOverflow,
  IncompleteFrame = impl::kCobsStatusIncompleteFrame,
};

inline constexpr size_t MaxEncodeLen(size_t decode_len) {
  return COBS_MAX_ENCODE_LEN(decode_len);
}

inline constexpr size_t MaxDecodeLen(size_t encode_len) {
  return COBS_MAX_DECODE_LEN(encode_len);
}

inline size_t Encode(uint8_t *output_buf, const uint8_t *input_buf, size_t input_len) {
  return impl::CobsEncodeBuffer(output_buf, input_buf, input_len);
}

inline std::vector<uint8_t> Encode(const uint8_t *input_buf, size_t input_len) {
  std::vector<uint8_t> output(MaxEncodeLen(input_len));
  size_t output_len = impl::CobsEncodeBuffer(output.data(), input_buf, input_len);
  output.resize(output_len);
  return output;
}

class Encoder {
 public:
  // Non-allocating constructor.  output_buf must be large enough to hold entire encoded frame.
  // No overflow checking is performed.  See MaxEncodeLen()
  Encoder(uint8_t *output_buf) : buf_ptr_{output_buf} { Reset(); }

  // Allocating constructor.  output_buf_size must be large enough to hold entire encoded frame.
  // No overflow checking is performed.  See MaxEncodeLen()
  Encoder(size_t output_buf_size) : buf_{new uint8_t[output_buf_size]}, buf_ptr_{buf_.get()} {
    Reset();
  }

  // Resets encoder.
  void Reset() { impl::CobsEncodeStateInit(&state_, buf_ptr_); }

  // Incrementally encode buffer.
  void Encode(const uint8_t *input_buf, size_t input_len) {
    impl::CobsEncodeBlock(&state_, input_buf, input_len, false);
  }

  // Get pointer to encoded buffer and reset the encoder.  The pointer is valid until the next call
  // to Encode().
  std::pair<const uint8_t *, size_t> Get() {
    impl::CobsEncodeBlock(&state_, nullptr, 0, true);
    auto output = std::make_pair(state_.encoded, state_.len);
    Reset();
    return output;
  }

  // Get copy of encoded buffer and reset the encoder.
  std::vector<uint8_t> GetCopy() {
    impl::CobsEncodeBlock(&state_, nullptr, 0, true);
    std::vector<uint8_t> output(state_.encoded, state_.encoded + state_.len);
    Reset();
    return output;
  }

 private:
  impl::CobsEncodeState state_;
  std::unique_ptr<uint8_t[]> buf_;
  uint8_t *const buf_ptr_;
};

inline Status Decode(uint8_t *output_buf, size_t *output_len, const uint8_t *input_buf,
                     size_t input_len) {
  const Status status =
      static_cast<Status>(impl::CobsDecodeBuffer(output_buf, output_len, input_buf, input_len));
  return status;
}

inline std::pair<Status, std::vector<uint8_t>> Decode(const uint8_t *input_buf, size_t input_len) {
  // Create buffer same size as input instead of MaxDecodeLen(input_len) so that incomplete frames
  // are flagged correctly instead of as overflows.
  std::vector<uint8_t> output(input_len);
  size_t output_len = output.size();
  const Status status =
      static_cast<Status>(impl::CobsDecodeBuffer(output.data(), &output_len, input_buf, input_len));
  output.resize(output_len);
  return {status, output};
}

class Decoder {
 public:
  Decoder(uint8_t *output_buf, size_t output_buf_len)
      : buf_ptr_{output_buf}, buf_len_{output_buf_len} {
    Reset();
  }

  Decoder(size_t output_buf_len)
      : buf_{new uint8_t[output_buf_len]}, buf_ptr_{buf_.get()}, buf_len_{output_buf_len} {
    Reset();
  }

  void Reset() { impl::CobsDecodeStateInit(&state_, buf_ptr_, buf_len_); }

  std::pair<Status, std::pair<uint8_t *, size_t>> Decode(uint8_t byte) {
    Status status = static_cast<Status>(impl::CobsDecodeByte(&state_, byte));
    if (status != Status::FrameAvailable) {
      return {status, {nullptr, 0}};
    }
    return {status, {state_.decoded, state_.len}};
  }

  std::pair<Status, std::vector<uint8_t>> DecodeAndCopy(uint8_t byte) {
    Status status = static_cast<Status>(impl::CobsDecodeByte(&state_, byte));
    if (status != Status::FrameAvailable) {
      return {status, {}};
    }
    return {status, {state_.decoded, state_.decoded + state_.len}};
  }

 private:
  impl::CobsDecodeState state_;
  std::unique_ptr<uint8_t[]> buf_;
  uint8_t *const buf_ptr_;
  const size_t buf_len_;
};

}  // namespace cobs
