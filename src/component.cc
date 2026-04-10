#include "src/internal/component.h"

#include <vector>

#include "src/internal/nvboard_internal.h"
#include "src/internal/pins_internal.h"

namespace nvboard {

Component::Component(SDL_Renderer *rend, int cnt, int init_val,
                     ComponentType ct)
    : renderer_(rend), component_type_(ct), state_(init_val) {
  rects_.resize(cnt);
  textures_.resize(cnt);
}

bool Component::InRect(int x, int y) const {
  SDL_Rect *temp = rects_[0];
  return x >= temp->x && y >= temp->y && x < temp->x + temp->w &&
         y < temp->y + temp->h;
}

SDL_Renderer *Component::GetRenderer() const { return renderer_; }

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
  SDL_RenderCopy(renderer_, textures_[state_], nullptr, rects_[state_]);
  SetRedraw();
}

void Component::UpdateState() {
  uint16_t pin = *(pins_.begin());
  int newval = PinPeek(pin);
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

void InitLed(SDL_Renderer *renderer);
void InitSwitch(SDL_Renderer *renderer);
void InitButton(SDL_Renderer *renderer);
void InitSegs7(SDL_Renderer *renderer);
void InitKeyboard(SDL_Renderer *renderer);
void InitVga(SDL_Renderer *renderer);
void InitUart(SDL_Renderer *renderer);

void InitComponents(SDL_Renderer *renderer) {
  InitLed(renderer);
  InitSwitch(renderer);
  InitButton(renderer);
  InitSegs7(renderer);
  InitKeyboard(renderer);
  InitVga(renderer);
  InitUart(renderer);
}

namespace {

std::vector<Component *> &Components() {
  static std::vector<Component *> components;
  return components;
}

}  // namespace

std::vector<Component *> &GetComponents() { return Components(); }

void AddComponent(Component *c) { Components().push_back(c); }

void DeleteComponents() {
  for (auto comp_ptr : Components()) {
    comp_ptr->Remove();
    delete comp_ptr;
  }
  Components().clear();
}

void InitGui(SDL_Renderer * /*renderer*/) {
  for (auto ptr : Components()) {
    ptr->UpdateGui();
  }
}

void UpdateComponents(SDL_Renderer * /*renderer*/) {
  for (auto ptr : Components()) {
    ptr->UpdateState();
  }
}

}  // namespace nvboard
