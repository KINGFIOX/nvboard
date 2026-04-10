#ifndef __UART_H__
#define __UART_H__

#include <component.h>
#include <queue>
#include <string>
#include <term.h>

class UART : public Component {
private:
  Term *term;
  int tx_state, rx_state;
  uint16_t divisor;
  uint8_t tx_data, rx_data;
  std::string rx_sending_str;
  std::queue<uint8_t> rx_char_queue;
  bool need_update_gui;
  uint8_t *p_tx;

public:
  UART(SDL_Renderer *rend, int cnt, int init_val, int ct, int x, int y, int w,
       int h);
  ~UART();
  void set_divisor(uint16_t d);

  virtual void update_gui();
  virtual void update_state();
  void tx_receive();
  void rx_send();
  void rx_getchar(uint8_t ch);
  void term_focus(bool v);

  void direct_putchar(uint8_t ch);
  bool has_rx_char() const { return !rx_char_queue.empty(); }
  uint8_t get_rx_char();
};

#endif
