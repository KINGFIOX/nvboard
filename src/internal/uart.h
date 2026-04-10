#ifndef NVBOARD_SRC_INTERNAL_UART_H_
#define NVBOARD_SRC_INTERNAL_UART_H_

#include <cstdint>
#include <queue>
#include <string>

#include "src/internal/component.h"
#include "src/internal/term.h"

namespace nvboard {

class Uart : public Component {
 public:
  Uart(BoardImpl *board, int cnt, int init_val, ComponentType ct, int x, int y,
       int w, int h);
  ~Uart() override;
  void SetDivisor(uint16_t d);

  void UpdateGui() override;
  void UpdateState() override;
  void TxReceive();
  void RxSend();
  void RxGetchar(uint8_t ch);
  void TermFocus(bool v);

  void DirectPutchar(uint8_t ch);
  bool HasRxChar() const { return !rx_char_queue_.empty(); }
  uint8_t GetRxChar();

 private:
  Term *term_;
  int tx_state_;
  int rx_state_;
  uint16_t divisor_;
  uint8_t tx_data_;
  uint8_t rx_data_;
  std::string rx_sending_str_;
  std::queue<uint8_t> rx_char_queue_;
  bool need_update_gui_;
  uint8_t *p_tx_;
};

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_UART_H_
