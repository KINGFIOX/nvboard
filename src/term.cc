#include "src/internal/term.h"

#include <cstring>

#include "absl/log/absl_check.h"
#include "src/internal/font.h"
#include "src/internal/nvboard_internal.h"

namespace nvboard {

Term::Term(SDL_Renderer *r, int x, int y, int w, int h)
    : renderer_(r),
      cursor_x_(0),
      cursor_y_(0),
      is_cursor_visible_(true),
      is_focus_(false),
      screen_y_(0) {
  region_ = SDL_Rect{x, y, w, h};
  w_in_char_ = region_.w / kChWidth;
  h_in_char_ = region_.h / kChHeight;
  AddLine();
  cursor_texture_ = NewTexture(r, kChWidth, kChHeight, 0x10, 0x10, 0x10);
  focus_cursor_texture_ = NewTexture(r, kChWidth, kChHeight, 0xff, 0x00, 0xff);
  ClearScreen();
  DrawCursor();
  dirty_line_ = new bool[h_in_char_];
  dirty_char_ = new bool[w_in_char_ * h_in_char_];
  InitDirty(false);
}

void Term::InitDirty(bool val) {
  dirty_screen_ = val;
  std::memset(dirty_line_, val, h_in_char_);
  std::memset(dirty_char_, val, w_in_char_ * h_in_char_);
}

Term::~Term() {
  SDL_DestroyTexture(cursor_texture_);
  delete[] dirty_line_;
  delete[] dirty_char_;
  for (auto *line : lines_) {
    delete[] line;
  }
}

void Term::ClearScreen() { SDL_RenderFillRect(renderer_, &region_); }

void Term::SetCursorVisibility(bool v) {
  bool update_cursor_gui = is_cursor_visible_ ^ v;
  is_cursor_visible_ = v;
  if (update_cursor_gui) DrawCursor();
}

void Term::SetFocus(bool v) {
  bool update_cursor_gui = is_focus_ ^ v;
  is_focus_ = v;
  if (update_cursor_gui) DrawCursor();
}

void Term::Clear() {
  while (lines_.size() > 1) {
    delete[] lines_.back();
    lines_.pop_back();
  }
  std::memset(lines_[0], ' ', w_in_char_);
  cursor_x_ = cursor_y_ = screen_y_ = 0;
  ClearScreen();
  InitDirty(false);
  SetRedraw();
}

void Term::Newline() {
  cursor_x_ = 0;
  cursor_y_++;
  if (cursor_y_ >= static_cast<int>(lines_.size())) AddLine();
  if (cursor_y_ == screen_y_ + h_in_char_) {
    screen_y_++;
    InitDirty(true);
  }
}

void Term::CarriageReturn() { cursor_x_ = 0; }

uint8_t *Term::AddLine() {
  auto *l = new uint8_t[w_in_char_];
  std::memset(l, ' ', w_in_char_);
  lines_.push_back(l);
  return l;
}

void Term::SetDirtyChar(int y, int x) {
  dirty_char_[y * w_in_char_ + x] = true;
  dirty_line_[y] = true;
  dirty_screen_ = true;
}

void Term::Backspace(bool is_input) {
  if (cursor_x_ == 0) {
    if (!is_input) return;
    if (cursor_y_ == 0) return;
    delete[] lines_[cursor_y_];
    lines_.erase(lines_.begin() + cursor_y_);
    cursor_y_--;
    cursor_x_ = w_in_char_ - 1;
    if (cursor_y_ < screen_y_) {
      screen_y_--;
      InitDirty(true);
    }
  } else {
    if (IsCursorOnScreen()) SetDirtyChar(cursor_y_ - screen_y_, cursor_x_);
    cursor_x_--;
  }
  lines_[cursor_y_][cursor_x_] = ' ';
  if (IsCursorOnScreen()) SetDirtyChar(cursor_y_ - screen_y_, cursor_x_);
}

void Term::FeedCh(uint8_t ch) {
  if (ch >= 128) ch = '?';
  if (IsCursorOnScreen()) SetDirtyChar(cursor_y_ - screen_y_, cursor_x_);
  int y = cursor_y_;
  ABSL_CHECK_LT(y, static_cast<int>(lines_.size()));
  if (ch == '\n') {
    Newline();
    return;
  } else if (ch == '\r') {
    CarriageReturn();
    return;
  } else if (ch == '\b') {
    Backspace(false);
    return;
  }
  lines_[y][cursor_x_] = ch;
  cursor_x_++;
  if (cursor_x_ == w_in_char_) Newline();
}

void Term::FeedStr(const char *s) {
  while (*s != '\0') FeedCh(*(s++));
}

bool Term::IsCursorOnScreen() {
  return cursor_y_ >= screen_y_ && cursor_y_ < screen_y_ + h_in_char_;
}

void Term::DrawCursor() {
  if (IsCursorOnScreen()) {
    int y = cursor_y_ - screen_y_;
    int x = cursor_x_;
    SDL_Rect rect = region_;
    rect.w = kChWidth;
    rect.h = kChHeight;
    rect.y += kChHeight * y;
    rect.x += kChWidth * x;
    SDL_Texture *t = is_cursor_visible_
                         ? (is_focus_ ? focus_cursor_texture_ : cursor_texture_)
                         : Ch2TextureTerm(' ');
    SDL_RenderCopy(renderer_, t, nullptr, &rect);
    SetRedraw();
  }
}

void Term::UpdateGui() {
  if (!dirty_screen_) return;
  SDL_Rect rect = region_;
  rect.w = kChWidth;
  rect.h = kChHeight;
  for (int y = 0; y < h_in_char_; y++) {
    if (screen_y_ + y >= static_cast<int>(lines_.size())) break;
    if (!dirty_line_[y]) continue;

    uint8_t *l = lines_[screen_y_ + y];
    rect.y = region_.y + rect.h * y;
    bool *dirty = &dirty_char_[y * w_in_char_];
    for (int x = 0; x < w_in_char_; x++) {
      if (!dirty[x]) continue;

      uint8_t ch = l[x];
      rect.x = region_.x + rect.w * x;
      SDL_Texture *t = Ch2TextureTerm(ch);
      SDL_RenderCopy(renderer_, t, nullptr, &rect);
      SetRedraw();
    }
  }
  DrawCursor();
  InitDirty(false);
}

}  // namespace nvboard
