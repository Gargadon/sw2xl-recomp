#pragma once

#include <cstdint>

namespace sw2::native {
// Host values only. Guest byte order and addresses belong in the adapter.
struct PadSample {
  uint16_t buttons{};
  uint8_t left_trigger{}, right_trigger{};
  int16_t lx{}, ly{}, rx{}, ry{};
};

struct GameInput {
  uint32_t held{}, pressed{};
  int32_t lx{}, ly{}, rx{}, ry{};
  bool disconnected{};
};

// TU3 sub_82101888: triggers use strict >30, axes use signed >>8.
// Failure clears held/pressed/axes; reconnect treats held buttons as new.
constexpr GameInput ConvertInput(const PadSample& sample, uint32_t previous,
                                bool connected) {
  if (!connected) return {.disconnected = true};
  uint32_t held = sample.buttons;
  if (sample.left_trigger > 30) held |= 0x00010000;
  if (sample.right_trigger > 30) held |= 0x01000000;
  return {held, held & ~previous, sample.lx >> 8, sample.ly >> 8,
          sample.rx >> 8, sample.ry >> 8, false};
}
}  // namespace sw2::native
