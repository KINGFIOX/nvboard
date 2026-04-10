#include <cstdio>
#include <cstring>

#include "src/internal/nvboard_internal.h"

namespace nvboard {

namespace {

constexpr int kLedX = 60 + (4 / 2);
constexpr int kLedY = 360;
constexpr int kLedSep = 14;
constexpr int kLedWidth = 16;
constexpr int kLedHeight = 8;

void InitRenderLocal(SDL_Renderer *renderer) {
  constexpr int kGap = 12;
  int w = (kLedWidth + kLedSep) * 16 - kLedSep;
  SDL_Rect r = MakeRect(MakePoint(kLedX, kLedY) - MakePoint(kGap, kGap),
                         w + kGap * 2, 76 + kGap * 2);
  SDL_SetRenderDrawColor(renderer, 21, 153, 120, 0);
  SDL_RenderFillRect(renderer, &r);

  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);
  SDL_Point p[2];
  constexpr int kGap2 = kGap + 4;
  p[0] = MakePoint(kLedX, kLedY) + MakePoint(-kGap2, kLedHeight / 2);
  p[1] = p[0] + MakePoint(w + kGap2 * 2, 0);
  DrawThickerLine(renderer, p, 2);

  int w_group4 = (kLedWidth + kLedSep) * 4;
  p[0] = MakePoint(kLedX, kLedY) + MakePoint(w_group4, 0) -
         MakePoint(kLedSep / 2, 0) + MakePoint(0, kLedHeight / 2) +
         MakePoint(0, -kLedSep / 2);
  p[1] = p[0] + MakePoint(0, kLedSep);
  for (int i = 0; i < 3; i++) {
    DrawThickerLine(renderer, p, 2);
    p[0] = p[0] + MakePoint(w_group4, 0);
    p[1] = p[1] + MakePoint(w_group4, 0);
  }

  SDL_Point p0 = MakePoint(kLedX, kLedY) + MakePoint(w, 0) -
                 MakePoint(kLedWidth / 2, 0) + MakePoint(0, kLedHeight * 2);
  for (int i = 0; i < 16; i++) {
    char buf[8];
    int n = std::snprintf(buf, 8, "%d", i);
    DrawStr(renderer, buf, p0.x - kChWidth * n / 2, p0.y, 0xffffff);
    p0.x -= kLedWidth + kLedSep;
  }

  const char *str = "LED";
  p0 = MakePoint(kLedX, kLedY) - MakePoint(kGap2 + 4, 0) +
       MakePoint(0, kLedHeight / 2) - MakePoint(0, kChHeight / 2) -
       MakePoint(kChWidth * std::strlen(str), 0);
  DrawStr(renderer, str, p0.x, p0.y, 0xffffff);
}

}  // namespace

void InitLed(BoardImpl *impl) {
  SDL_Renderer *renderer = impl->renderer_;
  SDL_Texture *tled_off =
      NewTexture(renderer, kLedWidth, kLedHeight, 0x7f, 0x7f, 0x7f);
  SDL_Texture *tled_g =
      NewTexture(renderer, kLedWidth, kLedHeight, 0x00, 0xff, 0x00);
  InitRenderLocal(renderer);

  for (int i = 0; i < 16; ++i) {
    auto *ptr = new Component(impl, 2, 0, ComponentType::kNaiveLed);

    auto *rect_ptr = new SDL_Rect;
    *rect_ptr = SDL_Rect{kLedX + (15 - i) * (kLedWidth + kLedSep), kLedY,
                         kLedWidth, kLedHeight};
    ptr->SetRect(rect_ptr, 0);
    ptr->SetTexture(tled_off, 0);

    rect_ptr = new SDL_Rect;
    *rect_ptr = SDL_Rect{kLedX + (15 - i) * (kLedWidth + kLedSep), kLedY,
                         kLedWidth, kLedHeight};
    ptr->SetRect(rect_ptr, 1);
    ptr->SetTexture(tled_g, 1);

    ptr->AddPin(LD0 + i);
    impl->AddComponent(ptr);
  }
}

}  // namespace nvboard
