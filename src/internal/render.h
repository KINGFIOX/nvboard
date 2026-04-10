#ifndef NVBOARD_SRC_INTERNAL_RENDER_H_
#define NVBOARD_SRC_INTERNAL_RENDER_H_

#include <SDL.h>

namespace nvboard {

constexpr uint32_t kBoardBgColor = 0x00008060;

inline SDL_Point MakePoint(int x, int y) {
  return SDL_Point{x, y};
}

inline SDL_Point operator+(const SDL_Point &a, const SDL_Point &b) {
  return MakePoint(a.x + b.x, a.y + b.y);
}

inline SDL_Point operator-(const SDL_Point &a, const SDL_Point &b) {
  return MakePoint(a.x - b.x, a.y - b.y);
}

inline SDL_Rect MakeRect(int x, int y, int w, int h) {
  return SDL_Rect{x, y, w, h};
}

inline SDL_Rect MakeRect(const SDL_Point &top_left, int w, int h) {
  return MakeRect(top_left.x, top_left.y, w, h);
}

inline SDL_Rect MakeRect(const SDL_Point &top_left, const SDL_Point &size) {
  return MakeRect(top_left, size.x, size.y);
}

inline SDL_Rect operator+(const SDL_Rect &a, const SDL_Rect &b) {
  return MakeRect(a.x + b.x, a.y + b.y, a.w + b.w, a.h + b.h);
}

void DrawThickerLine(SDL_Renderer *renderer, const SDL_Point *points, int n);
void DrawSurroundingLine(SDL_Renderer *renderer, SDL_Rect r, int gap);
void DrawStr(SDL_Renderer *renderer, const char *str, int x, int y,
             uint32_t fg);
void DrawStr(SDL_Renderer *renderer, const char *str, int x, int y,
             uint32_t fg, uint32_t bg);

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_RENDER_H_
