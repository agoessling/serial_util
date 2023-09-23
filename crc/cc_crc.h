#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

extern "C" {
#include "crc/c_crc.h"
}

namespace crc {
namespace impl {

template <int N>
struct always_false : std::false_type {};

template <int N>
struct CrcConfig {
  static_assert(always_false<N>::value, "Unsupported number of bits N.");
};

template <>
struct CrcConfig<8> {
  using InfoType = Crc8Info;
  using ValueType = uint8_t;
};

template <>
struct CrcConfig<16> {
  using InfoType = Crc16Info;
  using ValueType = uint16_t;
};

template <>
struct CrcConfig<32> {
  using InfoType = Crc32Info;
  using ValueType = uint32_t;
};

}  // namespace impl

template <int N>
class Crc {
 public:
  using Info = typename impl::CrcConfig<N>::InfoType;
  using Value = typename impl::CrcConfig<N>::ValueType;

  static Value Block(const Info *info, const uint8_t *data, size_t len) {
    if constexpr (N == 8) {
      return Crc8Block(info, data, len);
    } else if constexpr (N == 16) {
      return Crc16Block(info, data, len);
    } else if constexpr (N == 32) {
      return Crc32Block(info, data, len);
    } else {
      static_assert(impl::always_false<N>::value, "Unsupported number of bits N.");
    }
  }

  Crc(const Info *info) : info_{info}, crc_{info_->initial_crc} {}

  Value Block(const uint8_t *data, size_t len) const { return Block(info_, data, len); }

  Value operator()(uint8_t byte) {
    if constexpr (N == 8) {
      crc_ = Crc8Update(info_, crc_, byte);
    } else if constexpr (N == 16) {
      crc_ = Crc16Update(info_, crc_, byte);
    } else if constexpr (N == 32) {
      crc_ = Crc32Update(info_, crc_, byte);
    } else {
      static_assert(impl::always_false<N>::value, "Unsupported number of bits N.");
    }

    return info_->final_xor ^ crc_;
  }

  Value operator()(const uint8_t *data, size_t len) {
    if constexpr (N == 8) {
      crc_ = Crc8Seq(info_, data, len, crc_);
    } else if constexpr (N == 16) {
      crc_ = Crc16Seq(info_, data, len, crc_);
    } else if constexpr (N == 32) {
      crc_ = Crc32Seq(info_, data, len, crc_);
    } else {
      static_assert(impl::always_false<N>::value, "Unsupported number of bits N.");
    }

    return info_->final_xor ^ crc_;
  }

  void Reset() { crc_ = info_->initial_crc; }

 private:
  const Info *const info_;
  Value crc_;
};

}  // namespace crc
