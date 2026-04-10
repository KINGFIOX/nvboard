#include <cstring>

#include "src/internal/nvboard_internal.h"

namespace nvboard {

namespace {

constexpr int kBtnCX = 520;
constexpr int kBtnCY = 250;
constexpr int kBtnCSep = 20;
constexpr int kBtnCWidth = 30;
constexpr int kBtnCHeight = 30;

const SDL_Rect kBtnRects[6] = {
    {kBtnCX, kBtnCY, kBtnCWidth, kBtnCHeight},
    {kBtnCX, kBtnCY - kBtnCHeight - kBtnCSep, kBtnCWidth, kBtnCHeight},
    {kBtnCX, kBtnCY + kBtnCHeight + kBtnCSep, kBtnCWidth, kBtnCHeight},
    {kBtnCX - kBtnCWidth - kBtnCSep, kBtnCY, kBtnCWidth, kBtnCHeight},
    {kBtnCX + kBtnCWidth + kBtnCSep, kBtnCY, kBtnCWidth, kBtnCHeight},
    {kBtnCX + kBtnCWidth + kBtnCSep / 2,
     kBtnCY - 2 * (kBtnCHeight + kBtnCSep), kBtnCWidth, kBtnCHeight},
};

void InitRenderLocal(SDL_Renderer *renderer) {
  constexpr int kGap = 8;
  int w = (kBtnCWidth + kBtnCSep) * 3 - kBtnCSep;
  DrawSurroundingLine(
      renderer, MakeRect(kBtnRects[3].x, kBtnRects[1].y, w, w), kGap);

  const char *str = "CUDLR";
  char buf[2] = "?";
  for (int i = 0; i < 5; i++) {
    SDL_Point p = MakePoint(kBtnRects[i].x, kBtnRects[i].y) +
                  MakePoint(kBtnCWidth + 2, kBtnCHeight / 2) -
                  MakePoint(0, kChHeight / 2);
    buf[0] = str[i];
    DrawStr(renderer, buf, p.x, p.y, 0xffffff, kBoardBgColor);
  }

  str = "Button Pad";
  SDL_Point p = MakePoint(kBtnRects[3].x, kBtnRects[1].y) -
                MakePoint(0, kGap) - MakePoint(0, kChHeight / 2) +
                MakePoint(w / 2, 0) -
                MakePoint(kChWidth * std::strlen(str) / 2, 0);
  DrawStr(renderer, str, p.x, p.y, 0xffffff, kBoardBgColor);
}

}  // namespace

void InitButton(SDL_Renderer *renderer) {
  SDL_Texture *tbutton_on = LoadPicTexture(renderer, kVbtnOnPath);
  SDL_Texture *tbutton_off = LoadPicTexture(renderer, kVbtnOffPath);
  InitRenderLocal(renderer);

  for (int i = 0; i < 5; ++i) {
    auto *ptr = new Component(renderer, 2, 0, ComponentType::kButton);

    auto *rect_ptr = new SDL_Rect;
    *rect_ptr = kBtnRects[i];
    ptr->SetRect(rect_ptr, 0);
    ptr->SetTexture(tbutton_off, 0);

    rect_ptr = new SDL_Rect;
    *rect_ptr = kBtnRects[i];
    ptr->SetRect(rect_ptr, 1);
    ptr->SetTexture(tbutton_on, 1);

    ptr->AddPin(BTNC + i);
    AddComponent(ptr);
  }
}

}  // namespace nvboard
