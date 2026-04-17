#ifndef NVBOARD_SRC_INTERNAL_UART_H_
#define NVBOARD_SRC_INTERNAL_UART_H_

#include <cstdint>
#include <queue>

#include "src/internal/component.h"
#include "src/internal/term.h"

namespace nvboard {

class Uart : public Component {
 public:
  Uart(BoardImpl *board, int cnt, int init_val, ComponentType ct, int x, int y,
       int w, int h);
  ~Uart() override;

  void UpdateGui() override;
  void UpdateState() override;
  void RxGetchar(uint8_t ch);
  void TermFocus(bool v);

  void DirectPutchar(uint8_t ch);
  bool HasRxChar() const { return !rx_char_queue_.empty(); }
  uint8_t GetRxChar();

 private:
  Term *term_;
  std::queue<uint8_t> rx_char_queue_;
  bool need_update_gui_;
};

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_UART_H_
