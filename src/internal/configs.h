#ifndef NVBOARD_SRC_INTERNAL_CONFIGS_H_
#define NVBOARD_SRC_INTERNAL_CONFIGS_H_

#include <cstdint>

namespace nvboard {

constexpr const char *kVbtnOnPath = "vbtn_on.png";
constexpr const char *kVbtnOffPath = "vbtn_off.png";
constexpr const char *kVswOnPath = "vsw_on.png";
constexpr const char *kVswOffPath = "vsw_off.png";

constexpr int kWindowWidth = 640 * 2;
constexpr int kWindowHeight = 480 * 2;

// #define HARDWARE_ACC
// #define VSYNC

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_CONFIGS_H_
