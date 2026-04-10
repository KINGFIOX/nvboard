#include <cstring>

#include "absl/log/absl_check.h"
#include "src/internal/nvboard_internal.h"

namespace nvboard {

namespace {

constexpr int kSegX = 60;
constexpr int kSegY = 225;

constexpr int kSegVerWidth = 3;
constexpr int kSegVerHeight = 30;
constexpr int kSegHorWidth = 30;
constexpr int kSegHorHeight = 3;
constexpr int kSegDotWidth = 4;
constexpr int kSegDotHeight = 4;

constexpr int kSegSep = 3;

constexpr int kSegTotWidth =
    kSegSep * 18 + kSegVerWidth * 16 + kSegDotWidth * 8 + kSegHorWidth * 8;
constexpr int kSegTotHeight =
    kSegSep * 4 + kSegVerHeight * 2 + kSegHorHeight * 3;

int GetSegA(int i) { return SEG0A + 8 * i; }
int GetDecP(int i) { return SEG0A + 8 * i + 7; }

SDL_Texture *tsegled_ver_off;
SDL_Texture *tsegled_ver_on;
SDL_Texture *tsegled_hor_off;
SDL_Texture *tsegled_hor_on;
SDL_Texture *tsegled_dot_off;
SDL_Texture *tsegled_dot_on;

const SDL_Rect kSegsRect[8] = {
    {kSegSep + kSegVerWidth, kSegSep, kSegHorWidth, kSegHorHeight},
    {kSegSep + kSegVerWidth + kSegHorWidth, kSegSep + kSegHorHeight,
     kSegVerWidth, kSegVerHeight},
    {kSegSep + kSegVerWidth + kSegHorWidth,
     kSegSep + 2 * kSegHorHeight + kSegVerHeight, kSegVerWidth, kSegVerHeight},
    {kSegSep + kSegVerWidth,
     kSegSep + 2 * kSegHorHeight + 2 * kSegVerHeight, kSegHorWidth,
     kSegHorHeight},
    {kSegSep, kSegSep + 2 * kSegHorHeight + kSegVerHeight, kSegVerWidth,
     kSegVerHeight},
    {kSegSep, kSegSep + kSegHorHeight, kSegVerWidth, kSegVerHeight},
    {kSegSep + kSegVerWidth, kSegSep + kSegHorHeight + kSegVerHeight,
     kSegHorWidth, kSegHorHeight},
    {2 * kSegSep + 2 * kSegVerWidth + kSegHorWidth,
     kSegSep + 3 * kSegHorHeight + 2 * kSegVerHeight - kSegDotHeight,
     kSegDotWidth, kSegDotHeight},
};

SDL_Texture *SegsTexture(int index, int val) {
  switch (index) {
    case 0:
    case 3:
    case 6:
      return val ? tsegled_hor_on : tsegled_hor_off;
    case 1:
    case 2:
    case 4:
    case 5:
      return val ? tsegled_ver_on : tsegled_ver_off;
    case 7:
      return val ? tsegled_dot_on : tsegled_dot_off;
    default:
      ABSL_CHECK(false) << "Invalid segment index: " << index;
      return nullptr;
  }
}

void InitRenderLocal(SDL_Renderer *renderer) {
  tsegled_ver_on =
      NewTexture(renderer, kSegVerWidth, kSegVerHeight, 0xff, 0x00, 0x00);
  tsegled_ver_off =
      NewTexture(renderer, kSegVerWidth, kSegVerHeight, 0x2b, 0x2b, 0x2b);

  tsegled_hor_on =
      NewTexture(renderer, kSegHorWidth, kSegHorHeight, 0xff, 0x00, 0x00);
  tsegled_hor_off =
      NewTexture(renderer, kSegHorWidth, kSegHorHeight, 0x2b, 0x2b, 0x2b);

  tsegled_dot_on =
      NewTexture(renderer, kSegDotWidth, kSegDotHeight, 0xff, 0x00, 0x00);
  tsegled_dot_off =
      NewTexture(renderer, kSegDotWidth, kSegDotHeight, 0x2b, 0x2b, 0x2b);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_Rect rect_seg7 = {kSegX, kSegY, kSegTotWidth, kSegTotHeight};
  SDL_RenderFillRect(renderer, &rect_seg7);
  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);

  constexpr int kGap = 14;
  DrawSurroundingLine(renderer,
                      MakeRect(kSegX, kSegY, kSegTotWidth, kSegTotHeight),
                      kGap);

  SDL_Point p = MakePoint(kSegX, kSegY) +
                MakePoint(kSegTotWidth, kSegTotHeight) + MakePoint(0, kGap) -
                MakePoint(kSegTotWidth / 8 / 2, 0) -
                MakePoint(kChWidth / 2, kChHeight / 2);
  char buf[2] = "?";
  for (int i = 0; i < 8; i++) {
    buf[0] = '0' + i;
    DrawStr(renderer, buf, p.x, p.y, 0xffffff, kBoardBgColor);
    p = p - MakePoint(kSegTotWidth / 8, 0);
  }

  const char *str = "Seven Segment Display";
  p = MakePoint(kSegX, kSegY) - MakePoint(0, kGap) -
      MakePoint(0, kChHeight / 2) + MakePoint(kSegTotWidth / 2, 0) -
      MakePoint(kChWidth * std::strlen(str) / 2, 0);
  DrawStr(renderer, str, p.x, p.y, 0xffffff, kBoardBgColor);
}

}  // namespace

Segs7::Segs7(BoardImpl *board, int cnt, int init_val, ComponentType ct,
             bool is_len8)
    : Component(board, cnt, init_val, ct), is_len8_(is_len8) {}

void Segs7::UpdateGui() {
  int newval = GetState();
  for (int i = 0; i < 8; ++i) {
    int texture_idx = (7 - i) * 2 + (((newval >> i) & 1) ? 0 : 1);
    SDL_RenderCopy(GetRenderer(), GetTexture(texture_idx), nullptr,
                   GetRect(texture_idx));
  }
  board_->SetRedraw();
}

void Segs7::UpdateState() {
  int newval = 0;
  if (is_len8_) {
    newval = board_->PinPeek8(GetPin());
  } else {
    for (int i = 0; i < 8; ++i) {
      newval |= (board_->PinPeek(GetPin(7 - i)) << i);
    }
  }
  if (newval != GetState()) {
    SetState(newval);
    UpdateGui();
  }
}

void InitSegs7(BoardImpl *impl) {
  SDL_Renderer *renderer = impl->renderer_;
  InitRenderLocal(renderer);
  for (int i = 0; i < 8; ++i) {
    SDL_Rect mv = {kSegX + kSegSep +
                       (7 - i) * (kSegHorWidth + kSegDotWidth +
                                  kSegVerWidth * 2 + kSegSep * 2),
                   kSegY + kSegSep, 0, 0};
    bool is_len8 = (impl->pin_array_[GetSegA(i)].vector_len == 8);
    auto *ptr = new Segs7(impl, 16, 0x5555, ComponentType::kSegs7, is_len8);
    for (int j = 0; j < 8; ++j) {
      auto *rect_ptr = new SDL_Rect;
      *rect_ptr = mv + kSegsRect[j];
      ptr->SetTexture(SegsTexture(j, 0), j << 1 | 0);
      ptr->SetRect(rect_ptr, j << 1 | 0);
      rect_ptr = new SDL_Rect;
      *rect_ptr = mv + kSegsRect[j];
      ptr->SetTexture(SegsTexture(j, 1), j << 1 | 1);
      ptr->SetRect(rect_ptr, j << 1 | 1);
    }

    for (int p = GetSegA(i); p <= GetDecP(i); p++) {
      ptr->AddPin(p);
    }
    impl->AddComponent(ptr);
  }
}

}  // namespace nvboard
