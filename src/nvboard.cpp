#include <keyboard.h>
#include <macro.h>
#include <nvboard.h>
#include <stdarg.h>
#include <uart.h>
#include <vga.h>

#define FPS 60

static SDL_Window *main_window = nullptr;
static SDL_Renderer *main_renderer = nullptr;
PinNode pin_array[NR_PINS];

static bool need_redraw = true;
void set_redraw() { need_redraw = true; }

void vga_update();
void kb_update();
void uart_tx_receive();
void uart_rx_send();

void nvboard_update() {
  extern uint8_t *vga_blank_n_ptr;
  if (*vga_blank_n_ptr)
    vga_update();

  extern bool is_kb_idle;
  if (unlikely(!is_kb_idle))
    kb_update();

  extern int16_t uart_divisor_cnt;
  extern bool is_uart_rx_idle;
  if (unlikely((--uart_divisor_cnt) < 0)) {
    uart_tx_receive();
    if (unlikely(!is_uart_rx_idle))
      uart_rx_send();
  }

  static uint64_t last = 0;
  static int cpf = 1; // count per frame
  static int cnt = 0;
  if (unlikely((--cnt) < 0)) {
    uint64_t now = nvboard_get_time();
    uint64_t diff = now - last;
    if (diff == 0)
      return;
    int cpf_new =
        ((uint64_t)cpf * 1000000) / ((uint64_t)diff * FPS); // adjust cpf
    cnt += cpf_new - cpf;
    cpf = cpf_new;
    if (diff > 1000000 / FPS) {
      last = now;
      cnt = cpf;

      void read_event();
      read_event();
      update_components(main_renderer);
      if (need_redraw) {
        SDL_RenderPresent(main_renderer);
        need_redraw = false;
      }
    }
  }
}

void nvboard_init(int vga_clk_cycle) {
  // init SDL and SDL_image
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  IMG_Init(IMG_INIT_PNG);
#if defined(__APPLE__)
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "gpu");
#endif

  main_window = SDL_CreateWindow("NVBoard " VERSION_STR, SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                 WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
  SDL_RenderFillRect(main_renderer, NULL);

  for (int i = 0; i < NR_PINS; i++) {
    if (pin_array[i].ptr == NULL)
      pin_array[i].ptr = &pin_array[i].data;
  }

  void init_font(SDL_Renderer * renderer);
  init_font(main_renderer);
  init_render(main_renderer);
  init_components(main_renderer);
  init_gui(main_renderer);

  void init_nvboard_timer();
  init_nvboard_timer();

  update_components(main_renderer);

  extern void vga_set_clk_cycle(int cycle);
  vga_set_clk_cycle(vga_clk_cycle);
}

void nvboard_quit() {
  delete_components();
  SDL_DestroyWindow(main_window);
  SDL_DestroyRenderer(main_renderer);
  IMG_Quit();
  SDL_Quit();
}

void nvboard_bind_pin(void *signal, int len, ...) {
  assert(len < 64);
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

// --- Virtual device API (for spike+nvboard, no pin bindings) ---

static KEYBOARD *nvboard_get_kb() {
  extern KEYBOARD *kb_instance;
  return kb_instance;
}

static UART *nvboard_get_uart() {
  extern UART *uart_instance;
  return uart_instance;
}

static VGA *nvboard_get_vga() {
  extern VGA *vga_instance;
  return vga_instance;
}

bool nvboard_kbd_available() {
  KEYBOARD *kb = nvboard_get_kb();
  return kb && kb->has_scancode();
}

uint8_t nvboard_kbd_dequeue() {
  KEYBOARD *kb = nvboard_get_kb();
  if (!kb)
    return 0;
  return kb->dequeue_scancode();
}

void nvboard_vga_set_framebuffer(uint32_t *fb, int w, int h) {
  VGA *vga = nvboard_get_vga();
  if (vga)
    vga->set_external_framebuffer(fb, w, h);
}

void nvboard_vga_sync() {
  VGA *vga = nvboard_get_vga();
  if (vga)
    vga->sync_from_framebuffer();
}

void nvboard_uart_putchar(uint8_t ch) {
  UART *u = nvboard_get_uart();
  if (u)
    u->direct_putchar(ch);
}

bool nvboard_uart_available() {
  UART *u = nvboard_get_uart();
  return u && u->has_rx_char();
}

uint8_t nvboard_uart_getchar() {
  UART *u = nvboard_get_uart();
  if (!u)
    return 0;
  return u->get_rx_char();
}

uint16_t nvboard_sw_read() {
  uint16_t val = 0;
  for (int i = 0; i < 16; i++) {
    val |= (uint16_t)(pin_peek(SW0 + i) & 1) << i;
  }
  return val;
}

uint8_t nvboard_btn_read() {
  uint8_t val = 0;
  for (int i = 0; i < 5; i++) {
    val |= (uint8_t)(pin_peek(BTNC + i) & 1) << i;
  }
  return val;
}

void nvboard_led_write(uint16_t val) {
  for (int i = 0; i < 16; i++) {
    pin_poke(LD0 + i, (val >> i) & 1);
  }
}

void nvboard_seg7_write(int idx, uint8_t segments) {
  int base = SEG0A + idx * 8;
  for (int i = 0; i < 8; i++) {
    pin_poke(base + i, (segments >> i) & 1);
  }
}
