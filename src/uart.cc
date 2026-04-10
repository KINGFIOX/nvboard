#include "src/internal/uart.h"

#include "absl/log/absl_check.h"
#include "src/internal/board_impl.h"
#include "src/internal/nvboard_internal.h"

namespace nvboard {

namespace {

constexpr int kUartTxFps = 5;

}  // namespace

Uart::Uart(BoardImpl *board, int cnt, int init_val, ComponentType ct, int x,
           int y, int w, int h)
    : Component(board, cnt, init_val, ct),
      tx_state_(0),
      rx_state_(0),
      divisor_(16),
      tx_data_(0),
      rx_data_(0),
      need_update_gui_(false) {
  term_ = new Term(board, x, y, w, h);

  auto *rect_ptr = new SDL_Rect;
  *rect_ptr = SDL_Rect{x, y, w, h};
  SetRect(rect_ptr, 0);

  board_->uart_divisor_cnt_ = divisor_ - 1;
  int len = board_->pin_array_[UART_TX].vector_len;
  ABSL_CHECK(len == 0 || len == 1);
  p_tx_ = static_cast<uint8_t *>(board_->pin_array_[UART_TX].ptr);

  SDL_SetRenderDrawColor(board->renderer_, 0x00, 0x00, 0x00, 0);
  SDL_RenderDrawLine(board->renderer_, x, y + h, x + w, y + h);
  SDL_SetRenderDrawColor(board->renderer_, 0xff, 0xff, 0xff, 0);

  rx_sending_str_ = "";
  board_->PinPoke(UART_RX, 1);
}

Uart::~Uart() { SDL_DestroyTexture(GetTexture(0)); }

void Uart::UpdateGui() {}

void Uart::TxReceive() {
  board_->uart_divisor_cnt_ = divisor_ - 1;

  uint8_t tx = *p_tx_;
  if (tx_state_ == 0) {
    if (!tx) {
      tx_data_ = 0;
      tx_state_++;
    }
  } else if (tx_state_ >= 1 && tx_state_ <= 8) {
    tx_data_ = (tx << 7) | (tx_data_ >> 1);
    tx_state_++;
  } else if (tx_state_ == 9) {
    if (tx) {
      tx_state_ = 0;
      term_->FeedCh(tx_data_);
      need_update_gui_ = true;
    }
  }
}

void Uart::RxSend() {
  if (rx_state_ == 0) {
    rx_data_ = rx_sending_str_[0];
    if (rx_data_ == '\0') {
      board_->is_uart_rx_idle_ = true;
      return;
    }
    rx_sending_str_.erase(0, 1);
    board_->PinPoke(UART_RX, 0);
    rx_state_++;
  } else if (rx_state_ >= 1 && rx_state_ <= 8) {
    board_->PinPoke(UART_RX, rx_data_ & 1);
    rx_data_ >>= 1;
    rx_state_++;
  } else if (rx_state_ == 9) {
    board_->PinPoke(UART_RX, 1);
    rx_state_ = 0;
  }
}

void Uart::RxGetchar(uint8_t ch) {
  rx_sending_str_ += ch;
  rx_char_queue_.push(ch);
  board_->is_uart_rx_idle_ = false;
}

void Uart::DirectPutchar(uint8_t ch) {
  term_->FeedCh(ch);
  need_update_gui_ = true;
}

uint8_t Uart::GetRxChar() {
  if (rx_char_queue_.empty()) return 0;
  uint8_t ch = rx_char_queue_.front();
  rx_char_queue_.pop();
  return ch;
}

void Uart::UpdateState() {
  if (need_update_gui_) {
    static uint64_t last = 0;
    uint64_t now = board_->GetTime();
    if (now - last > 1000000 / kUartTxFps) {
      last = now;
      need_update_gui_ = false;
      term_->UpdateGui();
    }
  }
}

void Uart::SetDivisor(uint16_t d) { divisor_ = d; }

void Uart::TermFocus(bool v) { term_->SetFocus(v); }

namespace {

void InitRenderLocal(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);
  SDL_Point p[2];
  p[0] = MakePoint(kWindowWidth / 2, kWindowHeight / 2) - MakePoint(0, 30);
  p[1] = p[0] - MakePoint(10, 0);
  const char *label[2] = {"RX", "TX"};
  for (int i = 0; i < 2; i++) {
    DrawThickerLine(renderer, p, 2);
    DrawStr(renderer, label[i], p[1].x - 2 * kChWidth,
            p[1].y - kChHeight / 2, 0xffffff);
    p[0].y -= kChHeight;
    p[1].y -= kChHeight;
  }
  DrawStr(renderer, "UART-[", p[1].x - 8 * kChWidth, p[1].y + kChHeight,
          0xffffff);
}

}  // namespace

void InitUart(BoardImpl *impl) {
  InitRenderLocal(impl->renderer_);
  int x = kWindowWidth / 2, y = 0, w = kWindowWidth / 2, h = kWindowHeight / 2;
  impl->uart_device_ =
      new Uart(impl, 1, 0, ComponentType::kUart, x, y, w, h);
  impl->uart_device_->AddPin(UART_TX);
  impl->uart_device_->AddPin(UART_RX);
  impl->AddComponent(impl->uart_device_);
}

}  // namespace nvboard
