#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bat {

struct ButtonSequence {
  // Gaps shorter than DEBOUNCE_THRESHOLD will be ignored
  //   (instead the preceding hold will be extended)
  static constexpr uint32_t DEBOUNCE_THRESHOLD = 50;

  // Presses shorter than TAP_THRESHOLD will be reported as a
  // TAP, otherwise they will be a HOLD
  static constexpr uint32_t TAP_THRESHOLD = 1000;

  // Gaps longer than TIMEOUT will separate sequences
  static constexpr uint32_t TIMEOUT = 1000;

  static constexpr char TAP = 't';
  static constexpr char HOLD = 'h';

  static constexpr uint8_t MAX_COUNT = 3;
  char presses[MAX_COUNT];
  uint8_t count = 0;

  struct {
    uint32_t pressed_at = -TIMEOUT, released_at = -TIMEOUT;
    bool is_pressed = false;
  } most_recent;

  char const *begin() const { return presses; }
  char const *end() const { return begin() + count; }

  void press(uint32_t now) {
    most_recent.is_pressed = true;

    auto length = now - most_recent.released_at;

    // check for a gap below the debounce threshold,
    // in which case we don't update pressed_at and walk back count
    if (length < DEBOUNCE_THRESHOLD) {
      --count;
      return;
    }

    // this is the start of a new hold
    most_recent.pressed_at = now;

    // check for a gap longer than the timeout threshold
    if (length > TIMEOUT) {
      // in which case we reset the count
      count = 0;
    }
  }

  void release(uint32_t now) {
    most_recent.is_pressed = false;
    most_recent.released_at = now;

    if (count == MAX_COUNT) {
      // Too many holds; rotate out the first hold to make room
      count = MAX_COUNT - 1;
      memmove(&presses, &presses[1], count);
    }

    auto length = now - most_recent.pressed_at;
    // push into holds, checking for a hold longer than the long press threshold
    presses[count++] = length > TAP_THRESHOLD ? HOLD : TAP;
  }

  bool ready(uint32_t now) {
    if (most_recent.is_pressed) return false;

    auto length = now - most_recent.released_at;
    // check for a gap longer than the timeout threshold
    return length > TIMEOUT;
  }
};

}  // namespace bat
