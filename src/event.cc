#include <cstdlib>
#include <vector>

#include <SDL.h>

#include "src/internal/component.h"
#include "src/internal/pins_internal.h"

namespace nvboard {

extern std::vector<Component *> &GetComponents();
void UartRxGetchar(uint8_t ch);
void UartTermFocus(bool v);
void KbPushKey(uint8_t scancode, bool is_keydown);

namespace {

bool uart_term_has_focus = false;

void MousedownHandler(const SDL_Event &ev) {
  int x_pos = ev.button.x;
  int y_pos = ev.button.y;
  bool click_uart_term = false;
  for (auto i : GetComponents()) {
    if (i->InRect(x_pos, y_pos)) {
      switch (i->GetComponentType()) {
        case ComponentType::kButton:
          PinPoke(i->GetPin(), 1);
          break;
        case ComponentType::kSwitch:
          PinPoke(i->GetPin(), i->GetState() ^ 1);
          break;
        case ComponentType::kUart:
          click_uart_term = true;
          break;
        default:
          break;
      }
    }
  }
  if (uart_term_has_focus ^ click_uart_term) {
    if (click_uart_term)
      SDL_StartTextInput();
    else
      SDL_StopTextInput();
    uart_term_has_focus = click_uart_term;
    UartTermFocus(uart_term_has_focus);
  }
}

void MouseupHandler(const SDL_Event &ev) {
  int x_pos = ev.button.x;
  int y_pos = ev.button.y;
  for (auto i : GetComponents()) {
    if (i->InRect(x_pos, y_pos)) {
      switch (i->GetComponentType()) {
        case ComponentType::kButton:
          PinPoke(i->GetPin(), 0);
          break;
        default:
          break;
      }
    }
  }
}

}  // namespace

void ReadEvent() {
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
      case SDL_QUIT:
        std::exit(0);
      case SDL_WINDOWEVENT:
        if (ev.window.event == SDL_WINDOWEVENT_CLOSE) {
          std::exit(0);
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        MousedownHandler(ev);
        break;
      case SDL_MOUSEBUTTONUP:
        MouseupHandler(ev);
        break;
      case SDL_KEYDOWN:
        if (uart_term_has_focus) {
          switch (ev.key.keysym.sym) {
            case SDLK_RETURN:
              UartRxGetchar('\n');
              break;
            case SDLK_BACKSPACE:
              UartRxGetchar('\b');
              break;
          }
        }
        [[fallthrough]];
      case SDL_KEYUP:
        if (!uart_term_has_focus)
          KbPushKey(ev.key.keysym.scancode, ev.key.type == SDL_KEYDOWN);
        break;
      case SDL_TEXTINPUT:
        if (uart_term_has_focus) UartRxGetchar(ev.text.text[0]);
        break;
    }
  }
}

}  // namespace nvboard
