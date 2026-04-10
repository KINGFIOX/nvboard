#include <cstring>

#include "src/internal/nvboard_internal.h"

namespace nvboard {

namespace {

constexpr int kSwitchX = 60;
constexpr int kSwitchY = 400;
constexpr int kSwitchSep = 10;
constexpr int kSwitchWidth = 20;
constexpr int kSwitchHeight = 40;

void InitRenderLocal(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);
  SDL_Point p[2];
  constexpr int kGap2 = 16;
  int w = (kSwitchWidth + kSwitchSep) * 16 - kSwitchSep;
  p[0] = MakePoint(kSwitchX, kSwitchY) +
         MakePoint(-kGap2, kSwitchHeight / 2);
  p[1] = p[0] + MakePoint(w + kGap2 * 2, 0);
  DrawThickerLine(renderer, p, 2);

  int w_group4 = (kSwitchWidth + kSwitchSep) * 4;
  p[0] = MakePoint(kSwitchX, kSwitchY) + MakePoint(w_group4, 0) -
         MakePoint(kSwitchSep / 2, 0) + MakePoint(0, kSwitchHeight / 2) +
         MakePoint(0, -kSwitchSep / 2);
  p[1] = p[0] + MakePoint(0, kSwitchSep);
  for (int i = 0; i < 3; i++) {
    DrawThickerLine(renderer, p, 2);
    p[0] = p[0] + MakePoint(w_group4, 0);
    p[1] = p[1] + MakePoint(w_group4, 0);
  }

  const char *str = "SW";
  SDL_Point p0 =
      MakePoint(kSwitchX, kSwitchY) - MakePoint(kGap2 + 4, 0) +
      MakePoint(0, kSwitchHeight / 2) - MakePoint(0, kChHeight / 2) -
      MakePoint(kChWidth * std::strlen(str), 0);
  DrawStr(renderer, str, p0.x, p0.y, 0xffffff);
}

}  // namespace

void InitSwitch(BoardImpl *impl) {
  SDL_Renderer *renderer = impl->renderer_;
  SDL_Texture *tswitch_on = LoadPicTexture(impl, kVswOnPath);
  SDL_Texture *tswitch_off = LoadPicTexture(impl, kVswOffPath);
  InitRenderLocal(renderer);
  for (int i = 0; i < 16; ++i) {
    auto *ptr = new Component(impl, 2, 0, ComponentType::kSwitch);

    auto *rect_ptr = new SDL_Rect;
    *rect_ptr =
        SDL_Rect{kSwitchX + (15 - i) * (kSwitchWidth + kSwitchSep),
                 kSwitchY, kSwitchWidth, kSwitchHeight};
    ptr->SetRect(rect_ptr, 0);
    ptr->SetTexture(tswitch_off, 0);

    rect_ptr = new SDL_Rect;
    *rect_ptr =
        SDL_Rect{kSwitchX + (15 - i) * (kSwitchWidth + kSwitchSep),
                 kSwitchY, kSwitchWidth, kSwitchHeight};
    ptr->SetRect(rect_ptr, 1);
    ptr->SetTexture(tswitch_on, 1);

    ptr->AddPin(SW0 + i);
    impl->AddComponent(ptr);
  }
}

}  // namespace nvboard
