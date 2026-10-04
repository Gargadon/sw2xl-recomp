#include "native/input_guest.h"
#include <cstdio>

using namespace sw2::native;
#define CHECK(condition) do { if (!(condition)) { \
  std::fprintf(stderr, "Failed at line %d: %s\n", __LINE__, #condition); \
  return 1; } } while (false)

int main() {
  PadSample sample{.buttons = 0x2100};
  auto state = ConvertInput(sample, 0, true);
  CHECK(state.held == 0x2100 && state.pressed == 0x2100);
  state = ConvertInput(sample, state.held, true);
  CHECK(state.pressed == 0 && !state.disconnected);
  sample.buttons = 0x1000;
  state = ConvertInput(sample, state.held, true);
  CHECK(state.held == 0x1000 && state.pressed == 0x1000);
  sample.buttons = 0;
  CHECK(ConvertInput(sample, state.held, true).held == 0);

  for (unsigned trigger = 0; trigger <= 255; ++trigger) {
    sample.left_trigger = static_cast<uint8_t>(trigger);
    sample.right_trigger = 0;
    CHECK(ConvertInput(sample, 0, true).held == (trigger > 30 ? 0x10000u : 0));
    sample.left_trigger = 0;
    sample.right_trigger = static_cast<uint8_t>(trigger);
    CHECK(ConvertInput(sample, 0, true).held == (trigger > 30 ? 0x1000000u : 0));
  }
  // Independent arithmetic oracle: floor division, including negative values
  // that C++ integer division would truncate toward zero.
  for (int axis = -32768; axis <= 32767; ++axis) {
    sample.lx = sample.ly = sample.rx = sample.ry = static_cast<int16_t>(axis);
    state = ConvertInput(sample, 0, true);
    const int expected = axis < 0 ? (axis - 255) / 256 : axis / 256;
    CHECK(state.lx == expected && state.ly == expected);
    CHECK(state.rx == expected && state.ry == expected);
  }
  sample = {0xFFFF, 255, 255, -32768, 32767, -1, 256};
  state = ConvertInput(sample, 0xFFFFFFFF, false);
  CHECK(state.disconnected && state.held == 0 && state.pressed == 0);
  CHECK(state.lx == 0 && state.ly == 0 && state.rx == 0 && state.ry == 0);
  state = ConvertInput(sample, state.held, true);
  CHECK(!state.disconnected && state.held == 0x0101FFFF);
  CHECK(state.pressed == state.held);
  CHECK(ConvertInput(sample, state.held, true).pressed == 0);

  // Reported regression: LB/X while the left stick is held diagonally.
  sample = {0, 0, 0, 32767, -32768, 0, 0};
  state = ConvertInput(sample, 0, true);
  sample.buttons = 0x4100;
  state = ConvertInput(sample, state.held, true);
  CHECK(state.held == 0x4100 && state.pressed == 0x4100);
  CHECK(state.lx == 127 && state.ly == -128);
  state = ConvertInput(sample, state.held, true);
  CHECK(state.pressed == 0);
  sample.buttons = 0;
  state = ConvertInput(sample, state.held, true);
  sample.buttons = 0x4100;
  state = ConvertInput(sample, state.held, true);
  CHECK(state.pressed == 0x4100 && state.lx == 127 && state.ly == -128);

  // Literal guest-layout fixture, independent of WriteInput's implementation.
  InputBytes before{};
  before.fill(0xA5);
  before[12] = before[13] = before[15] = 0;
  before[14] = 0x20;  // Previously held B.
  const auto original_before = before;
  sample = {0x2100, 31, 30, -32768, 32767, -1, 256};
  auto expected = InputExpectation::From(before, sample, true);
  CHECK(before == original_before);  // Shadow path cannot modify the game.
  const uint8_t fixture[] = {
      0x00, 0x01, 0x21, 0x00,  // held: LT + B + LB
      0x00, 0x01, 0x01, 0x00,  // pressed: LT + LB
      0xFF, 0xFF, 0xFF, 0x80,  // LX: -128
      0x00, 0x00, 0x00, 0x7F,  // LY: 127
      0xFF, 0xFF, 0xFF, 0xFF,  // RX: -1
      0x00, 0x00, 0x00, 0x01,  // RY: 1
      0x00};                   // connected
  InputBytes actual = before;
  std::copy(std::begin(fixture), std::end(fixture), actual.begin() + 12);
  CHECK(expected.Differences(actual, 1) == 0);
  CHECK(ReadInputWord(actual, 20) == 0xFFFFFF80);
  for (size_t byte = 0; byte < actual.size(); ++byte) {
    actual[byte] ^= 1;
    CHECK(expected.Differences(actual, 1) == (uint64_t{1} << byte));
    actual[byte] ^= 1;
  }
  CHECK(expected.Differences(actual, 0) == (uint64_t{1} << 37));
  CHECK(expected.Differences(actual, 2) == (uint64_t{1} << 37));
  expected = InputExpectation::From(before, sample, false);
  std::fill(actual.begin() + 12, actual.end(), 0);
  actual[36] = 1;
  CHECK(expected.Differences(actual, 0) == 0);
  actual[36] = 2;  // Detect incorrect nonzero flags, not just bool equality.
  CHECK(expected.Differences(actual, 0) == (uint64_t{1} << 36));

  // XL has the opposite connection flag. All other converted bytes agree.
  for (const bool connected : {false, true}) {
    const auto base_result = InputExpectation::From(before, sample, connected);
    const auto xl_result = InputExpectation::From(before, sample, connected, true);
    CHECK(base_result.result == xl_result.result);
    for (size_t i = 0; i < 36; ++i) CHECK(base_result.bytes[i] == xl_result.bytes[i]);
    CHECK(xl_result.bytes[36] == (connected ? 1 : 0));
    CHECK(xl_result.Differences(base_result.bytes, base_result.result) == (uint64_t{1} << 36));
  }

  // Writes must preserve the prefix and never extend past the known layout.
  std::array<uint8_t, kInputObjectSize + 2> guarded;
  guarded.fill(0x5A);
  WriteInput(std::span<uint8_t, kInputObjectSize>(guarded.data() + 1, kInputObjectSize), state);
  CHECK(guarded.front() == 0x5A && guarded.back() == 0x5A);
  for (size_t i = 1; i <= 12; ++i) CHECK(guarded[i] == 0x5A);
  std::puts("Native input: button transitions, trigger boundaries, all axes and reconnect passed.");
  std::puts("Guest ABI: byte order, write boundaries, preserved prefix and every mismatch bit passed.");
}
