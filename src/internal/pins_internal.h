#ifndef NVBOARD_SRC_INTERNAL_PINS_INTERNAL_H_
#define NVBOARD_SRC_INTERNAL_PINS_INTERNAL_H_

#include <cstdint>

#include "absl/log/absl_check.h"
#include "nvboard/pins.h"

namespace nvboard {

struct PinNode {
  void *ptr = nullptr;
  uint8_t data = 0;
  uint8_t vector_len = 0;
  uint8_t bit_offset = 0;
};

extern PinNode pin_array[];

inline uint8_t PinPeek(int pin) {
  PinNode *p = &pin_array[pin];
  if (p->vector_len == 1) {
    return *static_cast<uint8_t *>(p->ptr) & 1;
  }
  uint64_t v = *static_cast<uint64_t *>(p->ptr);
  return (v >> p->bit_offset) & 1;
}

inline uint8_t PinPeek8(int pin) {
  PinNode *p = &pin_array[pin];
  ABSL_CHECK_EQ(p->vector_len, 8);
  return *static_cast<uint8_t *>(p->ptr);
}

inline void PinPoke(int pin, uint64_t v) {
  PinNode *p = &pin_array[pin];
  if (p->vector_len == 1) {
    *static_cast<uint8_t *>(p->ptr) = v & 1;
  } else {
    uint64_t x = *static_cast<uint64_t *>(p->ptr);
    uint64_t mask = uint64_t{1} << p->bit_offset;
    *static_cast<uint64_t *>(p->ptr) =
        (x & ~mask) | ((v & 1) << p->bit_offset);
  }
}

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_PINS_INTERNAL_H_
