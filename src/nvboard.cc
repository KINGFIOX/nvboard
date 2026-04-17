#include "nvboard/nvboard.h"

#include "absl/base/optimization.h"
#include "absl/log/absl_check.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "src/internal/nvboard_internal.h"

namespace nvboard {

// ---------------------------------------------------------------------------
// BoardImpl non-inline methods
// ---------------------------------------------------------------------------

uint64_t BoardImpl::GetTime() const {
  return absl::ToInt64Microseconds(absl::Now() - boot_time_);
}

void BoardImpl::DeleteComponents() {
  for (auto *c : components_) {
    c->Remove();
    delete c;
  }
  components_.clear();
}

void BoardImpl::UpdateComponents() {
  for (auto *c : components_) {
    c->UpdateState();
  }
}

void BoardImpl::InitGui() {
  for (auto *c : components_) {
    c->UpdateGui();
  }
}

// ---------------------------------------------------------------------------
// Board
// ---------------------------------------------------------------------------

constexpr int kFps = 60;

Board::Board() = default;

Board::~Board() {
  if (!impl_) return;
  impl_->DeleteComponents();
  void CloseFont();
  CloseFont();
  if (impl_->renderer_) SDL_DestroyRenderer(impl_->renderer_);
  if (impl_->window_) SDL_DestroyWindow(impl_->window_);
  IMG_Quit();
  SDL_Quit();
}

std::unique_ptr<Board> Board::Create(int vga_clk_cycle) {
  auto board = std::unique_ptr<Board>(new Board());
  board->impl_ = std::make_unique<BoardImpl>();
  BoardImpl *impl = board->impl_.get();

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  IMG_Init(IMG_INIT_PNG);
#if defined(__APPLE__)
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "gpu");
#endif

  impl->window_ = SDL_CreateWindow(
      "NVBoard " NVBOARD_VERSION_STR, SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, kWindowWidth, kWindowHeight, SDL_WINDOW_SHOWN);
  impl->renderer_ = SDL_CreateRenderer(
      impl->window_, -1,
#ifdef VSYNC
      SDL_RENDERER_PRESENTVSYNC |
#endif
#ifdef HARDWARE_ACC
      SDL_RENDERER_ACCELERATED |
#else
      SDL_RENDERER_SOFTWARE |
#endif
          0);
  SDL_SetRenderDrawColor(impl->renderer_, 0xff, 0xff, 0xff, 0);
  SDL_RenderFillRect(impl->renderer_, nullptr);

  for (int i = 0; i < NR_PINS; i++) {
    if (impl->pin_array_[i].ptr == nullptr)
      impl->pin_array_[i].ptr = &impl->pin_array_[i].data;
  }

  void InitFont(BoardImpl *);
  InitFont(impl);
  InitRender(impl);
  InitComponents(impl);
  impl->InitGui();
  impl->boot_time_ = absl::Now();
  impl->UpdateComponents();
  impl->vga_clk_cycle_minus_1_ = vga_clk_cycle - 1;

  return board;
}

void Board::Update() {
  BoardImpl *impl = impl_.get();

  if (impl->vga_blank_n_ptr_ && *impl->vga_blank_n_ptr_)
    impl->vga_device_->UpdateState();

  if (ABSL_PREDICT_FALSE(!impl->is_kb_idle_))
    impl->keyboard_->UpdateState();

  if (ABSL_PREDICT_FALSE((--impl->frame_cnt_) < 0)) {
    uint64_t now = impl->GetTime();
    uint64_t diff = now - impl->frame_last_;
    if (diff == 0) return;
    int cpf_new =
        (static_cast<uint64_t>(impl->frame_cpf_) * 1000000) /
        (static_cast<uint64_t>(diff) * kFps);
    impl->frame_cnt_ += cpf_new - impl->frame_cpf_;
    impl->frame_cpf_ = cpf_new;
    if (diff > 1000000 / kFps) {
      impl->frame_last_ = now;
      impl->frame_cnt_ = impl->frame_cpf_;

      void ReadEvent(BoardImpl *);
      ReadEvent(impl);
      impl->UpdateComponents();
      if (impl->need_redraw_) {
        SDL_RenderPresent(impl->renderer_);
        impl->need_redraw_ = false;
      }
    }
  }
}

void Board::BindPin(void *signal, absl::Span<const int> pins) {
  int len = static_cast<int>(pins.size());
  ABSL_CHECK_LT(len, 64);
  for (int i = 0; i < len; i++) {
    impl_->pin_array_[pins[i]].ptr = signal;
    impl_->pin_array_[pins[i]].vector_len = len;
    impl_->pin_array_[pins[i]].bit_offset = len - 1 - i;
  }
}

Board::Kbd &Board::kbd() { return impl_->kbd_view_; }
Board::Vga &Board::vga() { return impl_->vga_view_; }
Board::Uart &Board::uart() { return impl_->uart_view_; }

uint16_t Board::SwRead() {
  uint16_t val = 0;
  for (int i = 0; i < 16; i++) {
    val |= static_cast<uint16_t>(impl_->PinPeek(SW0 + i) & 1) << i;
  }
  return val;
}

uint8_t Board::BtnRead() {
  uint8_t val = 0;
  for (int i = 0; i < 5; i++) {
    val |= static_cast<uint8_t>(impl_->PinPeek(BTNC + i) & 1) << i;
  }
  return val;
}

void Board::LedWrite(uint16_t val) {
  for (int i = 0; i < 16; i++) {
    impl_->PinPoke(LD0 + i, (val >> i) & 1);
  }
}

void Board::Seg7Write(int idx, uint8_t segments) {
  int base = SEG0A + idx * 8;
  for (int i = 0; i < 8; i++) {
    impl_->PinPoke(base + i, (segments >> i) & 1);
  }
}

// ---------------------------------------------------------------------------
// Board::Kbd
// ---------------------------------------------------------------------------

bool Board::Kbd::Available() {
  return impl_->keyboard_ && impl_->keyboard_->HasScancode();
}

uint8_t Board::Kbd::Dequeue() {
  if (!impl_->keyboard_) return 0;
  return impl_->keyboard_->DequeueScancode();
}

// ---------------------------------------------------------------------------
// Board::Vga
// ---------------------------------------------------------------------------

void Board::Vga::SetFramebuffer(uint32_t *fb, int w, int h) {
  if (impl_->vga_device_) impl_->vga_device_->SetExternalFramebuffer(fb, w, h);
}

void Board::Vga::Sync() {
  if (impl_->vga_device_) impl_->vga_device_->SyncFromFramebuffer();
}

// ---------------------------------------------------------------------------
// Board::Uart
// ---------------------------------------------------------------------------

void Board::Uart::Putchar(uint8_t ch) {
  if (impl_->uart_device_) impl_->uart_device_->DirectPutchar(ch);
}

bool Board::Uart::Available() {
  return impl_->uart_device_ && impl_->uart_device_->HasRxChar();
}

uint8_t Board::Uart::Getchar() {
  if (!impl_->uart_device_) return 0;
  return impl_->uart_device_->GetRxChar();
}

}  // namespace nvboard
