#ifndef NVBOARD_SRC_INTERNAL_TERM_H_
#define NVBOARD_SRC_INTERNAL_TERM_H_

#include <cstdint>
#include <vector>

#include <SDL.h>

namespace nvboard {

class BoardImpl;

class Term {
 public:
  Term(BoardImpl *board, int x, int y, int w, int h);
  ~Term();
  void FeedCh(uint8_t ch);
  void Backspace(bool is_input);
  void FeedStr(const char *s);
  void Clear();
  void SetCursorVisibility(bool v);
  void SetFocus(bool v);
  void UpdateGui();

 private:
  void ClearScreen();
  void Newline();
  void CarriageReturn();
  uint8_t *AddLine();
  void DrawCursor();
  bool IsCursorOnScreen();
  void SetDirtyChar(int y, int x);
  void InitDirty(bool val);

  BoardImpl *board_;
  SDL_Renderer *renderer_;
  SDL_Rect region_;
  int w_in_char_;
  int h_in_char_;
  std::vector<uint8_t *> lines_;
  int cursor_x_;
  int cursor_y_;
  bool is_cursor_visible_;
  bool is_focus_;
  SDL_Texture *cursor_texture_;
  SDL_Texture *focus_cursor_texture_;
  int screen_y_;
  bool dirty_screen_;
  bool *dirty_line_;
  bool *dirty_char_;
};

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_TERM_H_
