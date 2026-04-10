#ifndef NVBOARD_SRC_INTERNAL_KEYBOARD_H_
#define NVBOARD_SRC_INTERNAL_KEYBOARD_H_

#include <cstdint>
#include <queue>

#include "src/internal/component.h"

namespace nvboard {

constexpr int kNotAKey = -1;
constexpr int kClkNum = 10;
constexpr int kPs2Start = 0;
constexpr int kPs2Data0 = 1;
constexpr int kPs2Data7 = 8;
constexpr int kPs2Partial = 9;
constexpr int kPs2Stop = 10;

#define NVBOARD_UINT2_XOR(a) (((a) >> 1) ^ ((a) & 0b1))
#define NVBOARD_UINT4_XOR(a) NVBOARD_UINT2_XOR(((a) >> 2) ^ ((a) & 0b11))
#define NVBOARD_UINT8_XOR(a) NVBOARD_UINT4_XOR(((a) >> 4) ^ ((a) & 0b1111))

class Keyboard : public Component {
 public:
  Keyboard(SDL_Renderer *rend, int cnt, int init_val, ComponentType ct);
  ~Keyboard() override;
  void PushKey(uint8_t scancode, bool is_keydown);
  void UpdateState() override;

  bool HasScancode() const { return !virtual_keys_.empty(); }
  uint8_t DequeueScancode();

 private:
  std::queue<uint8_t> all_keys_;
  std::queue<uint8_t> virtual_keys_;
  int data_idx_;
  int left_clk_;
  int cur_key_;
};

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_KEYBOARD_H_
