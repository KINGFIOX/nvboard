#ifndef NVBOARD_SRC_INTERNAL_NVBOARD_INTERNAL_H_
#define NVBOARD_SRC_INTERNAL_NVBOARD_INTERNAL_H_

#include <cstdint>
#include <string>

#include <SDL.h>
#include <SDL_image.h>

#include "nvboard/nvboard.h"
#include "src/internal/component.h"
#include "src/internal/configs.h"
#include "src/internal/font.h"
#include "src/internal/pins_internal.h"
#include "src/internal/render.h"

namespace nvboard {

#define NVBOARD_VERSION_STR "v1.0 (2024.01.10)"

void SetRedraw();
uint64_t GetTime();

void InitRender(SDL_Renderer *renderer);
SDL_Texture *LoadPicTexture(SDL_Renderer *renderer, std::string path);
SDL_Texture *NewTexture(SDL_Renderer *renderer, int w, int h, int r, int g,
                        int b);

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_NVBOARD_INTERNAL_H_
