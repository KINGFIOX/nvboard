#include "src/internal/render.h"

#include <cstring>
#include <string>

#include "absl/log/absl_check.h"
#include "absl/strings/str_cat.h"
#include "src/internal/nvboard_internal.h"

namespace nvboard {

namespace {

std::string nvboard_home;

SDL_Texture *LoadTexture(SDL_Renderer *renderer, const std::string &path) {
  SDL_Texture *t =
      IMG_LoadTexture(renderer, absl::StrCat(nvboard_home, path).c_str());
  ABSL_CHECK(t != nullptr) << "Failed to load texture: " << path;
  return t;
}

void DrawStrInternal(SDL_Renderer *renderer, SDL_Texture *t, const char *str,
                     int x, int y) {
  SDL_Rect r = MakeRect(x, y, kChWidth * std::strlen(str), kChHeight);
  SDL_RenderCopy(renderer, t, nullptr, &r);
  SDL_DestroyTexture(t);
}

}  // namespace

SDL_Texture *LoadPicTexture(SDL_Renderer *renderer, std::string path) {
  return LoadTexture(renderer, absl::StrCat("/resources/pic/", path));
}

SDL_Texture *Surface2Texture(SDL_Renderer *renderer, SDL_Surface *s) {
  ABSL_CHECK(s != nullptr);
  SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
  ABSL_CHECK(t != nullptr);
  SDL_FreeSurface(s);
  return t;
}

SDL_Texture *NewTexture(SDL_Renderer *renderer, int w, int h, int r, int g,
                        int b) {
  SDL_Surface *s = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  SDL_FillRect(s, nullptr, SDL_MapRGB(s->format, r, g, b));
  return Surface2Texture(renderer, s);
}

void DrawThickerLine(SDL_Renderer *renderer, const SDL_Point *points, int n) {
  SDL_RenderDrawLines(renderer, points, n);

  auto *p = new SDL_Point[n];
  for (int i = 0; i < n; i++) {
    p[i] = points[i] + MakePoint(-1, -1);
  }
  SDL_RenderDrawLines(renderer, &p[0], n);
  delete[] p;
}

void DrawSurroundingLine(SDL_Renderer *renderer, SDL_Rect r, int gap) {
  SDL_Point top_left = MakePoint(r.x, r.y) + MakePoint(-gap, -gap);
  const int w = r.w + gap * 2;
  const int h = r.h + gap * 2;
  const int d = 12;
  SDL_Point p[9];
  p[0] = top_left + MakePoint(d, 0);
  p[1] = p[0] + MakePoint(w - 2 * d, 0);
  p[2] = p[1] + MakePoint(d, d);
  p[3] = p[2] + MakePoint(0, h - 2 * d);
  p[4] = p[3] + MakePoint(-d, d);
  p[5] = p[4] - MakePoint(w - 2 * d, 0);
  p[6] = p[5] - MakePoint(d, d);
  p[7] = p[6] - MakePoint(0, h - 2 * d);
  p[8] = p[0];
  DrawThickerLine(renderer, p, 9);
}

void DrawStr(SDL_Renderer *renderer, const char *str, int x, int y,
             uint32_t fg) {
  SDL_Texture *t = Str2Texture(renderer, str, fg);
  DrawStrInternal(renderer, t, str, x, y);
}

void DrawStr(SDL_Renderer *renderer, const char *str, int x, int y,
             uint32_t fg, uint32_t bg) {
  SDL_Texture *t = Str2Texture(renderer, str, fg, bg);
  DrawStrInternal(renderer, t, str, x, y);
}

void InitRender(SDL_Renderer *renderer) {
  const char *home = std::getenv("NVBOARD_HOME");
  ABSL_CHECK(home != nullptr) << "NVBOARD_HOME environment variable not set";
  nvboard_home = home;

  SDL_Rect rect_bg = {0, 0, kWindowWidth / 2, kWindowHeight / 2};
  SDL_SetRenderDrawColor(renderer, (kBoardBgColor >> 16) & 0xff,
                         (kBoardBgColor >> 8) & 0xff, kBoardBgColor & 0xff, 0);
  SDL_RenderFillRect(renderer, &rect_bg);

  extern SDL_Texture *nvboard_texture;
  int w, h;
  SDL_QueryTexture(nvboard_texture, nullptr, nullptr, &w, &h);
  SDL_Rect r = MakeRect(60, 140, w, h);
  SDL_RenderCopy(renderer, nvboard_texture, nullptr, &r);
  SDL_DestroyTexture(nvboard_texture);

  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);
  SDL_Point p[2];
  p[0] = MakePoint(60, 140 + h + 4);
  p[1] = MakePoint(kWindowWidth / 2 / 2 + 120, p[0].y);
  DrawThickerLine(renderer, p, 2);
  p[0].y += 4;
  p[1].y += 4;
  DrawThickerLine(renderer, p, 2);

  DrawStr(renderer, NVBOARD_VERSION_STR, 60 + w + kChWidth, 140 + h - kChHeight,
          0xffffff);
}

}  // namespace nvboard
