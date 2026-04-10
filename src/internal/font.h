#ifndef NVBOARD_SRC_INTERNAL_FONT_H_
#define NVBOARD_SRC_INTERNAL_FONT_H_

#include <cstdint>

#include <SDL.h>

namespace nvboard {

constexpr int kChWidth = 10;
constexpr int kChHeight = 16;

SDL_Surface *Str2Surface(const char *str, uint32_t fg);
SDL_Surface *Str2Surface(const char *str, uint32_t fg, uint32_t bg);
SDL_Texture *Str2Texture(SDL_Renderer *renderer, const char *str, uint32_t fg);
SDL_Texture *Str2Texture(SDL_Renderer *renderer, const char *str, uint32_t fg,
                         uint32_t bg);
SDL_Surface *Ch2Surface(uint8_t ch, uint32_t fg);
SDL_Surface *Ch2Surface(uint8_t ch, uint32_t fg, uint32_t bg);
SDL_Texture *Ch2Texture(SDL_Renderer *renderer, uint8_t ch, uint32_t fg);
SDL_Texture *Ch2Texture(SDL_Renderer *renderer, uint8_t ch, uint32_t fg,
                        uint32_t bg);
SDL_Texture *Ch2TextureTerm(uint8_t ch);

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_FONT_H_
