#pragma once

#include "input.h"
#include <array>
#include <algorithm>
#include <span>

namespace sw2::native {
// Known prefix of TU3's input object; no host alignment assumptions.
inline constexpr size_t kInputObjectSize = 37;
using InputBytes = std::array<uint8_t, kInputObjectSize>;
using InputView = std::span<const uint8_t, kInputObjectSize>;

inline uint32_t ReadInputWord(InputView bytes, size_t offset) {
  return (uint32_t(bytes[offset]) << 24) | (uint32_t(bytes[offset + 1]) << 16) |
         (uint32_t(bytes[offset + 2]) << 8) | bytes[offset + 3];
}

inline void WriteInput(std::span<uint8_t, kInputObjectSize> bytes,
                       const GameInput& state, bool connected_flag = false) {
  const uint32_t words[] = {state.held, state.pressed,
      static_cast<uint32_t>(state.lx), static_cast<uint32_t>(state.ly),
      static_cast<uint32_t>(state.rx), static_cast<uint32_t>(state.ry)};
  for (size_t i = 0; i < 6; ++i) {
    for (size_t byte = 0; byte < 4; ++byte) {
      bytes[12 + i * 4 + byte] = static_cast<uint8_t>(words[i] >> (24 - byte * 8));
    }
  }
  // TU3 stores disconnected; XL stores connected at the same offset.
  bytes[36] = connected_flag ? !state.disconnected : state.disconnected;
}

// Pure expected-result builder also used by the live shadow verifier.
struct InputExpectation {
  InputBytes bytes{};
  uint32_t result{};

  static InputExpectation From(InputView before, const PadSample& sample,
                               bool connected, bool connected_flag = false) {
    InputExpectation expected;
    std::copy(before.begin(), before.end(), expected.bytes.begin());
    WriteInput(expected.bytes, ConvertInput(sample, ReadInputWord(before, 12), connected), connected_flag);
    expected.result = connected ? 1 : 0;
    return expected;
  }

  // One bit per differing byte, plus bit 37 for the function's return value.
  uint64_t Differences(InputView actual, uint32_t actual_result) const {
    uint64_t mask = actual_result == result ? 0 : (uint64_t{1} << kInputObjectSize);
    for (size_t i = 0; i < bytes.size(); ++i) {
      if (bytes[i] != actual[i]) mask |= uint64_t{1} << i;
    }
    return mask;
  }
};
}  // namespace sw2::native
