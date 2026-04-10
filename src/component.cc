#include "src/internal/component.h"

#include "src/internal/board_impl.h"

namespace nvboard {

Component::Component(BoardImpl *board, int cnt, int init_val, ComponentType ct)
    : board_(board), component_type_(ct), state_(init_val) {
  rects_.resize(cnt);
  textures_.resize(cnt);
}

bool Component::InRect(int x, int y) const {
  SDL_Rect *temp = rects_[0];
  return x >= temp->x && y >= temp->y && x < temp->x + temp->w &&
         y < temp->y + temp->h;
}

SDL_Renderer *Component::GetRenderer() const { return board_->renderer_; }

ComponentType Component::GetComponentType() const { return component_type_; }

SDL_Rect *Component::GetRect(int idx) const { return rects_[idx]; }

SDL_Texture *Component::GetTexture(int idx) const { return textures_[idx]; }

int Component::GetState() const { return state_; }

uint16_t Component::GetPin(int idx) const { return pins_[idx]; }

void Component::SetRect(SDL_Rect *rect, int val) { rects_[val] = rect; }

void Component::SetTexture(SDL_Texture *texture, int val) {
  textures_[val] = texture;
}

void Component::SetState(int val) { state_ = val; }

void Component::AddPin(uint16_t pin) { pins_.push_back(pin); }

void Component::UpdateGui() {
  SDL_RenderCopy(board_->renderer_, textures_[state_], nullptr,
                 rects_[state_]);
  board_->SetRedraw();
}

void Component::UpdateState() {
  uint16_t pin = *(pins_.begin());
  int newval = board_->PinPeek(pin);
  if (newval != state_) {
    SetState(newval);
    UpdateGui();
  }
}

void Component::Remove() {
  for (auto rect_ptr : rects_) {
    delete rect_ptr;
  }
}

void InitLed(BoardImpl *impl);
void InitSwitch(BoardImpl *impl);
void InitButton(BoardImpl *impl);
void InitSegs7(BoardImpl *impl);
void InitKeyboard(BoardImpl *impl);
void InitVga(BoardImpl *impl);
void InitUart(BoardImpl *impl);

void InitComponents(BoardImpl *impl) {
  InitLed(impl);
  InitSwitch(impl);
  InitButton(impl);
  InitSegs7(impl);
  InitKeyboard(impl);
  InitVga(impl);
  InitUart(impl);
}

}  // namespace nvboard
