#include "src/internal/uart.h"

#include "src/internal/board_impl.h"
#include "src/internal/configs.h"
#include "src/internal/font.h"
#include "src/internal/render.h"

namespace nvboard {

namespace {

constexpr int kUartTxFps = 5;

}  // namespace

Uart::Uart(BoardImpl *board, int cnt, int init_val, ComponentType ct, int x,
           int y, int w, int h)
    : Component(board, cnt, init_val, ct),
      need_update_gui_(false) {
  term_ = new Term(board, x, y, w, h);

  auto *rect_ptr = new SDL_Rect;
  *rect_ptr = SDL_Rect{x, y, w, h};
  SetRect(rect_ptr, 0);

  SDL_SetRenderDrawColor(board->renderer_, 0x00, 0x00, 0x00, 0);
  SDL_RenderDrawLine(board->renderer_, x, y + h, x + w, y + h);
  SDL_SetRenderDrawColor(board->renderer_, 0xff, 0xff, 0xff, 0);
}

Uart::~Uart() { SDL_DestroyTexture(GetTexture(0)); }

void Uart::UpdateGui() {}

void Uart::RxGetchar(uint8_t ch) { rx_char_queue_.push(ch); }

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
  impl->AddComponent(impl->uart_device_);
}

}  // namespace nvboard
