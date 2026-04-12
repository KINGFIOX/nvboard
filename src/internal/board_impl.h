#ifndef NVBOARD_SRC_INTERNAL_BOARD_IMPL_H_
#define NVBOARD_SRC_INTERNAL_BOARD_IMPL_H_

#include <cstdint>
#include <vector>

#include <SDL.h>

#include "absl/log/absl_check.h"
#include "absl/time/time.h"

#include "nvboard/nvboard.h"
#include "src/internal/component.h"
#include "src/internal/keyboard.h"
#include "src/internal/pins_internal.h"
#include "src/internal/uart.h"
#include "src/internal/vga.h"

namespace nvboard {

class BoardImpl {
 public:
  SDL_Window *window_ = nullptr;
  SDL_Renderer *renderer_ = nullptr;

  PinNode pin_array_[NR_PINS] = {};

  bool need_redraw_ = true;
  void SetRedraw() { need_redraw_ = true; }

  absl::Time boot_time_;
  uint64_t frame_last_ = 0;
  int frame_cpf_ = 1;
  int frame_cnt_ = 0;

  uint64_t GetTime() const;

  std::vector<Component *> components_;
  void AddComponent(Component *c) { components_.push_back(c); }
  void DeleteComponents();
  void UpdateComponents();
  void InitGui();

  Keyboard *keyboard_ = nullptr;
  bool is_kb_idle_ = true;
  Key keys_[256] = {};

  Vga *vga_device_ = nullptr;
  uint8_t *vga_blank_n_ptr_ = nullptr;
  int vga_clk_cycle_minus_1_ = 0;

  Uart *uart_device_ = nullptr;
  int16_t uart_divisor_cnt_ = 0;
  bool is_uart_rx_idle_ = true;

  SDL_Texture *nvboard_texture_ = nullptr;

  bool uart_term_has_focus_ = false;

  Board::Kbd kbd_view_{this};
  Board::Vga vga_view_{this};
  Board::Uart uart_view_{this};

  uint8_t PinPeek(int pin) const {
    const PinNode *p = &pin_array_[pin];
    if (p->vector_len == 1) {
      return *static_cast<uint8_t *>(p->ptr) & 1;
    }
    uint64_t v = *static_cast<uint64_t *>(p->ptr);
    return (v >> p->bit_offset) & 1;
  }

  uint8_t PinPeek8(int pin) const {
    const PinNode *p = &pin_array_[pin];
    ABSL_CHECK_EQ(p->vector_len, 8);
    return *static_cast<uint8_t *>(p->ptr);
  }

  void PinPoke(int pin, uint64_t v) {
    PinNode *p = &pin_array_[pin];
    if (p->vector_len == 1) {
      *static_cast<uint8_t *>(p->ptr) = v & 1;
    } else {
      uint64_t x = *static_cast<uint64_t *>(p->ptr);
      uint64_t mask = uint64_t{1} << p->bit_offset;
      *static_cast<uint64_t *>(p->ptr) =
          (x & ~mask) | ((v & 1) << p->bit_offset);
    }
  }
};

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_BOARD_IMPL_H_
