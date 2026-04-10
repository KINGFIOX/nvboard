#include "src/internal/keyboard.h"

#include <cstring>

#include "absl/log/absl_check.h"
#include "src/internal/at_scancode.h"
#include "src/internal/board_impl.h"
#include "src/internal/font.h"
#include "src/internal/nvboard_internal.h"

namespace nvboard {

namespace {

SDL_Surface *NewKeyShape(int w, int h) {
  SDL_Surface *s = SDL_CreateRGBSurface(0, w, h, 32, 0xff0000, 0x00ff00,
                                        0x0000ff, 0xff000000);
  uint32_t black = SDL_MapRGBA(s->format, 0, 0, 0, 0xff);
  SDL_Rect r;
  r = SDL_Rect{0, 0, 1, h};
  SDL_FillRect(s, &r, black);
  r = SDL_Rect{0, 0, w, 1};
  SDL_FillRect(s, &r, black);
  r = SDL_Rect{w - 1, 0, 1, h};
  SDL_FillRect(s, &r, black);
  r = SDL_Rect{0, h - 1, w, 1};
  SDL_FillRect(s, &r, black);
  return s;
}

SDL_Surface *SurfaceDup(SDL_Surface *src, uint32_t bg) {
  SDL_PixelFormat *f = src->format;
  SDL_Surface *s = SDL_CreateRGBSurface(0, src->w, src->h, f->BitsPerPixel,
                                        f->Rmask, f->Gmask, f->Bmask,
                                        f->Amask);
  SDL_FillRect(s, nullptr, bg);
  SDL_BlitSurface(src, nullptr, s, nullptr);
  return s;
}

SDL_Texture *GenKeyTexture(SDL_Renderer *renderer, const char *desc1,
                           const char *desc2, SDL_Surface *key_shape,
                           bool is_down) {
  std::string desc = std::string(desc1) + '\n' + desc2;
  uint32_t color_up = SDL_MapRGBA(key_shape->format, 0xf0, 0xf0, 0xf0, 0xff);
  uint32_t color_dn = SDL_MapRGBA(key_shape->format, 0xc0, 0xc0, 0xc0, 0xff);
  SDL_Surface *s = SurfaceDup(key_shape, is_down ? color_dn : color_up);
  SDL_Surface *s_desc = Str2Surface(desc.c_str(), 0);
  SDL_Rect r = SDL_Rect{1, 1, 0, 0};
  SDL_BlitSurface(s_desc, nullptr, s, &r);
  SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
  SDL_FreeSurface(s_desc);
  SDL_FreeSurface(s);
  return t;
}

void InitKeyTexture(BoardImpl *impl, uint8_t sdl_key, const char *desc1,
                    const char *desc2, SDL_Surface *key_shape, int x, int y) {
  Key *e = &impl->keys_[sdl_key];
  e->t_up = GenKeyTexture(impl->renderer_, desc1, desc2, key_shape, false);
  e->t_down = GenKeyTexture(impl->renderer_, desc1, desc2, key_shape, true);
  e->rect = SDL_Rect{x, y, key_shape->w, key_shape->h};
}

void InitRenderLocal(BoardImpl *impl) {
  SDL_Renderer *renderer = impl->renderer_;
  constexpr int kKeyUnitWidth = 34;
  constexpr int kKeyGap = kKeyUnitWidth / 14;
  constexpr int kHKeyboard =
      kKeyUnitWidth * 6 + kKeyGap * 5 + kKeyUnitWidth / 2;
  const int x_top_left = kWindowWidth / 2 + 1;
  const int y_below_uart = kWindowHeight / 2;
  const int y_top_left =
      y_below_uart + (kWindowHeight - y_below_uart - kHKeyboard) / 2;
  constexpr int kKeyGapBeforeExtKeys = kKeyUnitWidth / 3;

  SDL_Surface *s_1p0 = NewKeyShape(kKeyUnitWidth, kKeyUnitWidth);
  SDL_Surface *s_1p5 =
      NewKeyShape(kKeyUnitWidth + kKeyGap + kKeyUnitWidth / 2, kKeyUnitWidth);
  SDL_Surface *s_2p0 =
      NewKeyShape(kKeyUnitWidth * 2 + kKeyGap, kKeyUnitWidth);
  SDL_Surface *s_2p25 = NewKeyShape(
      kKeyUnitWidth * 2 + kKeyGap * 2 + kKeyUnitWidth / 4, kKeyUnitWidth);
  SDL_Surface *s_6p0 =
      NewKeyShape(kKeyUnitWidth * 6 + kKeyGap * 5, kKeyUnitWidth);

  int x = x_top_left, y = y_top_left;

#define NVBOARD_KEY_ENTRY(k, desc1, desc2, width)                              \
  do {                                                                         \
    int idx = SDL_PREFIX(NVBOARD_CONCAT(_, k));                                \
    InitKeyTexture(impl, idx, desc1, desc2, NVBOARD_CONCAT(s_, width), x, y);  \
    x += NVBOARD_CONCAT(s_, width)->w + kKeyGap;                              \
    SDL_RenderCopy(renderer, impl->keys_[idx].t_up, nullptr,                   \
                   &impl->keys_[idx].rect);                                    \
  } while (0)

#define NVBOARD_NEXTLINE()                                                     \
  do {                                                                         \
    x = x_top_left;                                                            \
    y += kKeyUnitWidth + kKeyGap;                                              \
  } while (0)

  NVBOARD_KEY_ENTRY(ESCAPE, "ESC", "", 1p0);
  x += kKeyUnitWidth / 2 + kKeyGap;
  NVBOARD_KEY_ENTRY(F1, "F1", "", 1p0);
  NVBOARD_KEY_ENTRY(F2, "F2", "", 1p0);
  NVBOARD_KEY_ENTRY(F3, "F3", "", 1p0);
  NVBOARD_KEY_ENTRY(F4, "F4", "", 1p0);
  x += kKeyUnitWidth / 2;
  NVBOARD_KEY_ENTRY(F5, "F5", "", 1p0);
  NVBOARD_KEY_ENTRY(F6, "F6", "", 1p0);
  NVBOARD_KEY_ENTRY(F7, "F7", "", 1p0);
  NVBOARD_KEY_ENTRY(F8, "F8", "", 1p0);
  x += kKeyUnitWidth / 2;
  NVBOARD_KEY_ENTRY(F9, "F9", "", 1p0);
  NVBOARD_KEY_ENTRY(F10, "F10", "", 1p0);
  NVBOARD_KEY_ENTRY(F11, "F11", "", 1p0);
  NVBOARD_KEY_ENTRY(F12, "F12", "", 1p0);
  x += kKeyGapBeforeExtKeys;
  NVBOARD_KEY_ENTRY(PRINTSCREEN, "Prt", "Scr", 1p0);
  NVBOARD_KEY_ENTRY(SCROLLLOCK, "Scr", "Lck", 1p0);
  NVBOARD_KEY_ENTRY(PAUSE, "Pa-", "use", 1p0);

  NVBOARD_NEXTLINE();
  y += kKeyUnitWidth / 2;
  NVBOARD_KEY_ENTRY(GRAVE, "~", "`", 1p0);
  NVBOARD_KEY_ENTRY(1, "!", "1", 1p0);
  NVBOARD_KEY_ENTRY(2, "@", "2", 1p0);
  NVBOARD_KEY_ENTRY(3, "#", "3", 1p0);
  NVBOARD_KEY_ENTRY(4, "$", "4", 1p0);
  NVBOARD_KEY_ENTRY(5, "%", "5", 1p0);
  NVBOARD_KEY_ENTRY(6, "^", "6", 1p0);
  NVBOARD_KEY_ENTRY(7, "&", "7", 1p0);
  NVBOARD_KEY_ENTRY(8, "*", "8", 1p0);
  NVBOARD_KEY_ENTRY(9, "(", "9", 1p0);
  NVBOARD_KEY_ENTRY(0, ")", "0", 1p0);
  NVBOARD_KEY_ENTRY(MINUS, "_", "-", 1p0);
  NVBOARD_KEY_ENTRY(EQUALS, "+", "=", 1p0);
  NVBOARD_KEY_ENTRY(BACKSPACE, "Back", "Space", 1p5);
  x += kKeyGapBeforeExtKeys - kKeyGap;
  NVBOARD_KEY_ENTRY(INSERT, "Ins", "", 1p0);
  NVBOARD_KEY_ENTRY(HOME, "Ho-", "me", 1p0);
  NVBOARD_KEY_ENTRY(PAGEUP, "Pg", "Up", 1p0);

  NVBOARD_NEXTLINE();
  NVBOARD_KEY_ENTRY(TAB, "Tab", "", 1p5);
  NVBOARD_KEY_ENTRY(Q, "Q", "", 1p0);
  NVBOARD_KEY_ENTRY(W, "W", "", 1p0);
  NVBOARD_KEY_ENTRY(E, "E", "", 1p0);
  NVBOARD_KEY_ENTRY(R, "R", "", 1p0);
  NVBOARD_KEY_ENTRY(T, "T", "", 1p0);
  NVBOARD_KEY_ENTRY(Y, "Y", "", 1p0);
  NVBOARD_KEY_ENTRY(U, "U", "", 1p0);
  NVBOARD_KEY_ENTRY(I, "I", "", 1p0);
  NVBOARD_KEY_ENTRY(O, "O", "", 1p0);
  NVBOARD_KEY_ENTRY(P, "P", "", 1p0);
  NVBOARD_KEY_ENTRY(LEFTBRACKET, "{", "[", 1p0);
  NVBOARD_KEY_ENTRY(RIGHTBRACKET, "}", "]", 1p0);
  NVBOARD_KEY_ENTRY(BACKSLASH, "|", "\\", 1p0);
  x += kKeyGapBeforeExtKeys - kKeyGap;
  NVBOARD_KEY_ENTRY(DELETE, "Del", "", 1p0);
  NVBOARD_KEY_ENTRY(END, "End", "", 1p0);
  NVBOARD_KEY_ENTRY(PAGEDOWN, "Pg", "Dn", 1p0);

  NVBOARD_NEXTLINE();
  NVBOARD_KEY_ENTRY(CAPSLOCK, "Caps", "Lock", 2p0);
  NVBOARD_KEY_ENTRY(A, "A", "", 1p0);
  NVBOARD_KEY_ENTRY(S, "S", "", 1p0);
  NVBOARD_KEY_ENTRY(D, "D", "", 1p0);
  NVBOARD_KEY_ENTRY(F, "F", "", 1p0);
  NVBOARD_KEY_ENTRY(G, "G", "", 1p0);
  NVBOARD_KEY_ENTRY(H, "H", "", 1p0);
  NVBOARD_KEY_ENTRY(J, "J", "", 1p0);
  NVBOARD_KEY_ENTRY(K, "K", "", 1p0);
  NVBOARD_KEY_ENTRY(L, "L", "", 1p0);
  NVBOARD_KEY_ENTRY(SEMICOLON, ":", ";", 1p0);
  NVBOARD_KEY_ENTRY(APOSTROPHE, "\"", "'", 1p0);
  NVBOARD_KEY_ENTRY(RETURN, "Enter", "", 1p5);

  NVBOARD_NEXTLINE();
  NVBOARD_KEY_ENTRY(LSHIFT, "Shift", "", 2p25);
  NVBOARD_KEY_ENTRY(Z, "Z", "", 1p0);
  NVBOARD_KEY_ENTRY(X, "X", "", 1p0);
  NVBOARD_KEY_ENTRY(C, "C", "", 1p0);
  NVBOARD_KEY_ENTRY(V, "V", "", 1p0);
  NVBOARD_KEY_ENTRY(B, "B", "", 1p0);
  NVBOARD_KEY_ENTRY(N, "N", "", 1p0);
  NVBOARD_KEY_ENTRY(M, "M", "", 1p0);
  NVBOARD_KEY_ENTRY(COMMA, "<", ",", 1p0);
  NVBOARD_KEY_ENTRY(PERIOD, ">", ".", 1p0);
  NVBOARD_KEY_ENTRY(SLASH, "?", "/", 1p0);
  NVBOARD_KEY_ENTRY(RSHIFT, "Shift", "", 2p25);
  x += kKeyGapBeforeExtKeys + kKeyUnitWidth;
  NVBOARD_KEY_ENTRY(UP, " ^", " |", 1p0);

  NVBOARD_NEXTLINE();
  NVBOARD_KEY_ENTRY(LCTRL, "Ctrl", "", 1p5);
  x += kKeyUnitWidth + kKeyGap + kKeyUnitWidth / 4;
  NVBOARD_KEY_ENTRY(LALT, "Alt", "", 1p5);
  NVBOARD_KEY_ENTRY(SPACE, "Space", "", 6p0);
  NVBOARD_KEY_ENTRY(RALT, "Alt", "", 1p5);
  x += kKeyUnitWidth + kKeyGap + kKeyUnitWidth / 4;
  NVBOARD_KEY_ENTRY(RCTRL, "Ctrl", "", 1p5);
  x += kKeyGapBeforeExtKeys - kKeyGap;
  NVBOARD_KEY_ENTRY(LEFT, "<-", "", 1p0);
  NVBOARD_KEY_ENTRY(DOWN, " |", " V", 1p0);
  NVBOARD_KEY_ENTRY(RIGHT, "->", "", 1p0);

#undef NVBOARD_KEY_ENTRY
#undef NVBOARD_NEXTLINE

  SDL_FreeSurface(s_1p0);
  SDL_FreeSurface(s_1p5);
  SDL_FreeSurface(s_2p0);
  SDL_FreeSurface(s_2p25);
  SDL_FreeSurface(s_6p0);

  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);
  SDL_Point p[3];
  p[0] = MakePoint(kWindowWidth / 2, kWindowHeight / 2);
  p[1] = p[0] - MakePoint(16, 16);
  p[2] = p[1] - MakePoint(20, 0);
  DrawThickerLine(renderer, p, 3);

  const char *str = "PS/2 Keyboard";
  DrawStr(renderer, str, p[2].x - std::strlen(str) * kChWidth,
          p[2].y - kChHeight / 2, 0xffffff);
}

}  // namespace

Keyboard::Keyboard(BoardImpl *board, int cnt, int init_val, ComponentType ct)
    : Component(board, cnt, init_val, ct),
      data_idx_(0),
      left_clk_(0),
      cur_key_(kNotAKey) {}

Keyboard::~Keyboard() = default;

uint8_t Keyboard::DequeueScancode() {
  if (virtual_keys_.empty()) return 0;
  uint8_t sc = virtual_keys_.front();
  virtual_keys_.pop();
  return sc;
}

void Keyboard::PushKey(uint8_t sdl_key, bool is_keydown) {
  Key *e = &board_->keys_[sdl_key];
  uint8_t at_key = e->map0;
  if (at_key == 0xe0) {
    all_keys_.push(0xe0);
    virtual_keys_.push(0xe0);
    at_key = e->map1;
  }
  if (!is_keydown) {
    all_keys_.push(0xf0);
    virtual_keys_.push(0xf0);
  }
  all_keys_.push(at_key);
  virtual_keys_.push(at_key);
  board_->is_kb_idle_ = false;

  if (e->pressing != is_keydown) {
    e->pressing = is_keydown;
    SDL_RenderCopy(GetRenderer(), (is_keydown ? e->t_down : e->t_up), nullptr,
                   &e->rect);
    board_->SetRedraw();
  }
}

void Keyboard::UpdateState() {
  if (cur_key_ == kNotAKey) {
    if (all_keys_.empty()) {
      board_->is_kb_idle_ = true;
      return;
    }
    cur_key_ = all_keys_.front();
    ABSL_CHECK_EQ(data_idx_, 0);
    left_clk_ = kClkNum;
  }

  if (left_clk_ == 0) {
    uint8_t ps2_clk = board_->PinPeek(PS2_CLK);
    ps2_clk = !ps2_clk;
    board_->PinPoke(PS2_CLK, ps2_clk);
    left_clk_ = kClkNum;
    if (ps2_clk) {
      ABSL_CHECK(!all_keys_.empty());
      uint8_t ps2_dat =
          (data_idx_ == kPs2Partial)
              ? !NVBOARD_UINT8_XOR(all_keys_.front())
              : (data_idx_ == kPs2Stop)
                    ? 1
                    : ((data_idx_ >= kPs2Data0) && (data_idx_ <= kPs2Data7))
                          ? (cur_key_ & 1)
                          : 0;
      board_->PinPoke(PS2_DAT, ps2_dat);
      if ((data_idx_ >= kPs2Data0) && (data_idx_ <= kPs2Data7))
        cur_key_ >>= 1;
      data_idx_++;
    } else if (data_idx_ == 11) {
      data_idx_ = 0;
      cur_key_ = kNotAKey;
      all_keys_.pop();
    }
  } else {
    left_clk_--;
  }
}

void InitKeyboard(BoardImpl *impl) {
  InitRenderLocal(impl);
  impl->keyboard_ = new Keyboard(impl, 0, 0, ComponentType::kKeyboard);
  for (int p = PS2_CLK; p <= PS2_DAT; p++) {
    impl->keyboard_->AddPin(p);
  }
#define FILL_KEYMAP0(a) impl->keys_[SDL_PREFIX(a)].map0 = GET_FIRST(AT_PREFIX(a));
#define FILL_KEYMAP1(a) impl->keys_[SDL_PREFIX(a)].map1 = GET_SECOND(AT_PREFIX(a));
  NVBOARD_MAP(SCANCODE_LIST, FILL_KEYMAP0)
  NVBOARD_MAP(SCANCODE_LIST, FILL_KEYMAP1)
#undef FILL_KEYMAP0
#undef FILL_KEYMAP1
}

}  // namespace nvboard
