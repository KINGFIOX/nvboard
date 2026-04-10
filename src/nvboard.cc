#include "nvboard/nvboard.h"

#include <cstdarg>

#include "absl/base/optimization.h"
#include "absl/log/absl_check.h"
#include "absl/log/initialize.h"
#include "src/internal/keyboard.h"
#include "src/internal/macro.h"
#include "src/internal/nvboard_internal.h"
#include "src/internal/uart.h"
#include "src/internal/vga.h"

namespace nvboard {

namespace {

constexpr int kFps = 60;

SDL_Window *main_window = nullptr;
SDL_Renderer *main_renderer = nullptr;

bool need_redraw = true;

}  // namespace

PinNode pin_array[NR_PINS];

void SetRedraw() { need_redraw = true; }

void VgaUpdate();
void KbUpdate();
void UartTxReceive();
void UartRxSend();
void InitTimer();

extern uint8_t *vga_blank_n_ptr;
extern bool &IsKbIdle();
extern int16_t uart_divisor_cnt;
extern bool is_uart_rx_idle;

void Update() {
  if (*vga_blank_n_ptr) VgaUpdate();

  if (ABSL_PREDICT_FALSE(!IsKbIdle())) KbUpdate();

  if (ABSL_PREDICT_FALSE((--uart_divisor_cnt) < 0)) {
    UartTxReceive();
    if (ABSL_PREDICT_FALSE(!is_uart_rx_idle)) UartRxSend();
  }

  static uint64_t last = 0;
  static int cpf = 1;
  static int cnt = 0;
  if (ABSL_PREDICT_FALSE((--cnt) < 0)) {
    uint64_t now = GetTime();
    uint64_t diff = now - last;
    if (diff == 0) return;
    int cpf_new =
        (static_cast<uint64_t>(cpf) * 1000000) /
        (static_cast<uint64_t>(diff) * kFps);
    cnt += cpf_new - cpf;
    cpf = cpf_new;
    if (diff > 1000000 / kFps) {
      last = now;
      cnt = cpf;

      void ReadEvent();
      ReadEvent();
      UpdateComponents(main_renderer);
      if (need_redraw) {
        SDL_RenderPresent(main_renderer);
        need_redraw = false;
      }
    }
  }
}

void InitFont(SDL_Renderer *renderer);
void VgaSetClkCycle(int cycle);

void Init(int vga_clk_cycle) {
  absl::InitializeLog();

  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  IMG_Init(IMG_INIT_PNG);
#if defined(__APPLE__)
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "gpu");
#endif

  main_window = SDL_CreateWindow("NVBoard " NVBOARD_VERSION_STR,
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, kWindowWidth,
                                 kWindowHeight, SDL_WINDOW_SHOWN);
  main_renderer = SDL_CreateRenderer(main_window, -1,
#ifdef VSYNC
                                     SDL_RENDERER_PRESENTVSYNC |
#endif
#ifdef HARDWARE_ACC
                                     SDL_RENDERER_ACCELERATED |
#else
                                     SDL_RENDERER_SOFTWARE |
#endif
                                         0);
  SDL_SetRenderDrawColor(main_renderer, 0xff, 0xff, 0xff, 0);
  SDL_RenderFillRect(main_renderer, nullptr);

  for (int i = 0; i < NR_PINS; i++) {
    if (pin_array[i].ptr == nullptr)
      pin_array[i].ptr = &pin_array[i].data;
  }

  InitFont(main_renderer);
  InitRender(main_renderer);
  InitComponents(main_renderer);
  InitGui(main_renderer);
  InitTimer();

  UpdateComponents(main_renderer);
  VgaSetClkCycle(vga_clk_cycle);
}

void Quit() {
  DeleteComponents();
  SDL_DestroyWindow(main_window);
  SDL_DestroyRenderer(main_renderer);
  IMG_Quit();
  SDL_Quit();
}

void BindPin(void *signal, int len, ...) {
  ABSL_CHECK_LT(len, 64);
  va_list ap;
  va_start(ap, len);
  for (int i = 0; i < len; i++) {
    uint16_t pin = va_arg(ap, int);
    pin_array[pin].ptr = signal;
    pin_array[pin].vector_len = len;
    pin_array[pin].bit_offset = len - 1 - i;
  }
  va_end(ap);
}

// --- Virtual device API ---

extern Keyboard *&GetKbInstance();
extern Uart *&GetUartInstance();
extern Vga *&GetVgaInstance();

bool KbdAvailable() {
  Keyboard *kb = GetKbInstance();
  return kb && kb->HasScancode();
}

uint8_t KbdDequeue() {
  Keyboard *kb = GetKbInstance();
  if (!kb) return 0;
  return kb->DequeueScancode();
}

void VgaSetFramebuffer(uint32_t *fb, int w, int h) {
  Vga *v = GetVgaInstance();
  if (v) v->SetExternalFramebuffer(fb, w, h);
}

void VgaSync() {
  Vga *v = GetVgaInstance();
  if (v) v->SyncFromFramebuffer();
}

void UartPutchar(uint8_t ch) {
  Uart *u = GetUartInstance();
  if (u) u->DirectPutchar(ch);
}

bool UartAvailable() {
  Uart *u = GetUartInstance();
  return u && u->HasRxChar();
}

uint8_t UartGetchar() {
  Uart *u = GetUartInstance();
  if (!u) return 0;
  return u->GetRxChar();
}

uint16_t SwRead() {
  uint16_t val = 0;
  for (int i = 0; i < 16; i++) {
    val |= static_cast<uint16_t>(PinPeek(SW0 + i) & 1) << i;
  }
  return val;
}

uint8_t BtnRead() {
  uint8_t val = 0;
  for (int i = 0; i < 5; i++) {
    val |= static_cast<uint8_t>(PinPeek(BTNC + i) & 1) << i;
  }
  return val;
}

void LedWrite(uint16_t val) {
  for (int i = 0; i < 16; i++) {
    PinPoke(LD0 + i, (val >> i) & 1);
  }
}

void Seg7Write(int idx, uint8_t segments) {
  int base = SEG0A + idx * 8;
  for (int i = 0; i < 8; i++) {
    PinPoke(base + i, (segments >> i) & 1);
  }
}

}  // namespace nvboard
