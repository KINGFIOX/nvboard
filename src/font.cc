#include "src/internal/font.h"

#include <cstring>
#include <string>

#include <SDL_ttf.h>

#include "absl/log/absl_check.h"
#include "absl/strings/str_cat.h"
#include "src/internal/configs.h"
#include "src/internal/render.h"

namespace nvboard {

namespace {

TTF_Font *font = nullptr;
SDL_Texture *font_texture_term[128] = {nullptr};

}  // namespace

SDL_Texture *Surface2Texture(SDL_Renderer *renderer, SDL_Surface *s);
SDL_Texture *nvboard_texture = nullptr;

void InitFont(SDL_Renderer *renderer) {
  int ret = TTF_Init();
  ABSL_CHECK_NE(ret, -1);
  const char *home = std::getenv("NVBOARD_HOME");
  ABSL_CHECK(home != nullptr) << "NVBOARD_HOME environment variable not set";
  std::string font_path =
      absl::StrCat(home, "/resources/font/FreeMono.ttf");

  font = TTF_OpenFont(font_path.c_str(), 48);
  ABSL_CHECK(font != nullptr) << "Failed to open font: " << font_path;
  TTF_SetFontHinting(font, TTF_HINTING_MONO);
  TTF_SetFontStyle(font, TTF_STYLE_BOLD);
  nvboard_texture =
      Str2Texture(renderer, "NVBoard", 0xffffff, kBoardBgColor);

  TTF_SetFontSize(font, kChHeight);
  SDL_Color fg = {0x00, 0x00, 0x00, 0xff};
  SDL_Color bg = {0xff, 0xff, 0xff, 0xff};
  for (int i = 1; i < 128; i++) {
    SDL_Surface *s = TTF_RenderGlyph_Shaded(font, i, fg, bg);
    ABSL_CHECK_EQ(s->w, kChWidth);
    ABSL_CHECK_EQ(s->h, kChHeight);
    font_texture_term[i] = Surface2Texture(renderer, s);
  }
}

SDL_Surface *Str2Surface(const char *str, uint32_t fg) {
  SDL_Color c_fg = {static_cast<uint8_t>(fg >> 16),
                    static_cast<uint8_t>(fg >> 8),
                    static_cast<uint8_t>(fg), 0xff};
  SDL_Surface *s = TTF_RenderText_Blended_Wrapped(font, str, c_fg, 0);
  ABSL_CHECK(s != nullptr);
  return s;
}

SDL_Surface *Str2Surface(const char *str, uint32_t fg, uint32_t bg) {
  SDL_Color c_fg = {static_cast<uint8_t>(fg >> 16),
                    static_cast<uint8_t>(fg >> 8),
                    static_cast<uint8_t>(fg), 0xff};
  SDL_Color c_bg = {static_cast<uint8_t>(bg >> 16),
                    static_cast<uint8_t>(bg >> 8),
                    static_cast<uint8_t>(bg), 0xff};
  SDL_Surface *s = TTF_RenderText_Shaded_Wrapped(font, str, c_fg, c_bg, 0);
  ABSL_CHECK(s != nullptr);
  return s;
}

SDL_Texture *Str2Texture(SDL_Renderer *renderer, const char *str,
                         uint32_t fg) {
  return Surface2Texture(renderer, Str2Surface(str, fg));
}

SDL_Texture *Str2Texture(SDL_Renderer *renderer, const char *str, uint32_t fg,
                         uint32_t bg) {
  return Surface2Texture(renderer, Str2Surface(str, fg, bg));
}

SDL_Surface *Ch2Surface(uint8_t ch, uint32_t fg) {
  SDL_Color c_fg = {static_cast<uint8_t>(fg >> 16),
                    static_cast<uint8_t>(fg >> 8),
                    static_cast<uint8_t>(fg), 0xff};
  SDL_Surface *s = TTF_RenderGlyph_Blended(font, ch, c_fg);
  ABSL_CHECK(s != nullptr);
  return s;
}

SDL_Surface *Ch2Surface(uint8_t ch, uint32_t fg, uint32_t bg) {
  SDL_Color c_fg = {static_cast<uint8_t>(fg >> 16),
                    static_cast<uint8_t>(fg >> 8),
                    static_cast<uint8_t>(fg), 0xff};
  SDL_Color c_bg = {static_cast<uint8_t>(bg >> 16),
                    static_cast<uint8_t>(bg >> 8),
                    static_cast<uint8_t>(bg), 0xff};
  SDL_Surface *s = TTF_RenderGlyph_Shaded(font, ch, c_fg, c_bg);
  ABSL_CHECK(s != nullptr);
  return s;
}

SDL_Texture *Ch2Texture(SDL_Renderer *renderer, uint8_t ch, uint32_t fg) {
  return Surface2Texture(renderer, Ch2Surface(ch, fg));
}

SDL_Texture *Ch2Texture(SDL_Renderer *renderer, uint8_t ch, uint32_t fg,
                        uint32_t bg) {
  return Surface2Texture(renderer, Ch2Surface(ch, fg, bg));
}

SDL_Texture *Ch2TextureTerm(uint8_t ch) {
  ABSL_CHECK_LT(ch, 128);
  return font_texture_term[ch == 0 ? ' ' : ch];
}

void CloseFont() {
  TTF_CloseFont(font);
  TTF_Quit();
}

}  // namespace nvboard
