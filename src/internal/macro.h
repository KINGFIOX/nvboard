#ifndef NVBOARD_SRC_INTERNAL_MACRO_H_
#define NVBOARD_SRC_INTERNAL_MACRO_H_

#include "absl/base/optimization.h"

#define NVBOARD_CONCAT_INNER(x, y) x##y
#define NVBOARD_CONCAT(x, y) NVBOARD_CONCAT_INNER(x, y)

#define VGA_POS_EDGE(signal)                                    \
  (NVBOARD_CONCAT(vga_, signal) == 1 &&                         \
   NVBOARD_CONCAT(vga_pre_, signal) == 0)
#define VGA_NEG_EDGE(signal)                                    \
  (NVBOARD_CONCAT(vga_, signal) == 0 &&                         \
   NVBOARD_CONCAT(vga_pre_, signal) == 1)

#endif  // NVBOARD_SRC_INTERNAL_MACRO_H_
