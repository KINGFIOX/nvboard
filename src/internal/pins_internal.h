#ifndef NVBOARD_SRC_INTERNAL_PINS_INTERNAL_H_
#define NVBOARD_SRC_INTERNAL_PINS_INTERNAL_H_

#include <cstdint>

#include "nvboard/pins.h"

namespace nvboard {

struct PinNode {
  void *ptr = nullptr;
  uint8_t data = 0;
  uint8_t vector_len = 0;
  uint8_t bit_offset = 0;
};

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_PINS_INTERNAL_H_
