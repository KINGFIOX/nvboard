#ifndef NVBOARD_SRC_INTERNAL_NVBOARD_INTERNAL_H_
#define NVBOARD_SRC_INTERNAL_NVBOARD_INTERNAL_H_

#include <cstddef>

#include <SDL_image.h>

#include "src/internal/board_impl.h"
#include "src/internal/configs.h"
#include "src/internal/font.h"
#include "src/internal/render.h"

namespace nvboard {

#define NVBOARD_VERSION_STR "v1.0 (2024.01.10)"

void InitRender(BoardImpl *impl);
SDL_Texture *LoadTextureFromMemory(SDL_Renderer *renderer,
                                   const unsigned char *data, size_t size);
SDL_Texture *NewTexture(SDL_Renderer *renderer, int w, int h, int r, int g,
                        int b);

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_NVBOARD_INTERNAL_H_
