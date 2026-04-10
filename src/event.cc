#include <cstdlib>

#include <SDL.h>

#include "src/internal/board_impl.h"

namespace nvboard {

namespace {

void MousedownHandler(BoardImpl *impl, const SDL_Event &ev) {
  int x_pos = ev.button.x;
  int y_pos = ev.button.y;
  bool click_uart_term = false;
  for (auto i : impl->components_) {
    if (i->InRect(x_pos, y_pos)) {
      switch (i->GetComponentType()) {
        case ComponentType::kButton:
          impl->PinPoke(i->GetPin(), 1);
          break;
        case ComponentType::kSwitch:
          impl->PinPoke(i->GetPin(), i->GetState() ^ 1);
          break;
        case ComponentType::kUart:
          click_uart_term = true;
          break;
        default:
          break;
      }
    }
  }
  if (impl->uart_term_has_focus_ ^ click_uart_term) {
    if (click_uart_term)
      SDL_StartTextInput();
    else
      SDL_StopTextInput();
    impl->uart_term_has_focus_ = click_uart_term;
    impl->uart_device_->TermFocus(impl->uart_term_has_focus_);
  }
}

void MouseupHandler(BoardImpl *impl, const SDL_Event &ev) {
  int x_pos = ev.button.x;
  int y_pos = ev.button.y;
  for (auto i : impl->components_) {
    if (i->InRect(x_pos, y_pos)) {
      switch (i->GetComponentType()) {
        case ComponentType::kButton:
          impl->PinPoke(i->GetPin(), 0);
          break;
        default:
          break;
      }
    }
  }
}

}  // namespace

void ReadEvent(BoardImpl *impl) {
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
        MousedownHandler(impl, ev);
        break;
      case SDL_MOUSEBUTTONUP:
        MouseupHandler(impl, ev);
        break;
      case SDL_KEYDOWN:
        if (impl->uart_term_has_focus_) {
          switch (ev.key.keysym.sym) {
            case SDLK_RETURN:
              impl->uart_device_->RxGetchar('\n');
              break;
            case SDLK_BACKSPACE:
              impl->uart_device_->RxGetchar('\b');
              break;
          }
        }
        [[fallthrough]];
      case SDL_KEYUP:
        if (!impl->uart_term_has_focus_)
          impl->keyboard_->PushKey(ev.key.keysym.scancode,
                                   ev.key.type == SDL_KEYDOWN);
        break;
      case SDL_TEXTINPUT:
        if (impl->uart_term_has_focus_)
          impl->uart_device_->RxGetchar(ev.text.text[0]);
        break;
    }
  }
}

}  // namespace nvboard
