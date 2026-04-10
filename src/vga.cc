#include "src/internal/vga.h"

#include <cstring>

#include "absl/base/optimization.h"
#include "absl/log/absl_check.h"
#include "src/internal/macro.h"
#include "src/internal/nvboard_internal.h"

namespace nvboard {

namespace {

Vga *vga_instance = nullptr;
Vga *&vga = vga_instance;

int vga_clk_cycle_minus_1 = 0;

}  // namespace

uint8_t *vga_blank_n_ptr = nullptr;

VgaMode vga_mod_accepted[kNrVgaMode] = {
    {96, 144, 784, 800, 2, 35, 515, 525},
};

Vga::Vga(SDL_Renderer *rend, int cnt, int init_val, ComponentType ct)
    : Component(rend, cnt, init_val, ct),
      vga_screen_width_(kVgaDefaultWidth),
      vga_screen_height_(kVgaDefaultHeight),
      vga_clk_cnt_(1),
      ext_fb_(nullptr),
      ext_fb_w_(0),
      ext_fb_h_(0) {
  SDL_Texture *vga_texture = SDL_CreateTexture(
      rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
      vga_screen_width_, vga_screen_height_);
  SetTexture(vga_texture, 0);
  pixels_ = new uint32_t[vga_screen_width_ * vga_screen_height_];
  std::memset(pixels_, 0,
              vga_screen_width_ * vga_screen_height_ * sizeof(uint32_t));

  auto *rect_ptr = new SDL_Rect;
  *rect_ptr = SDL_Rect{0, kWindowHeight / 2, kVgaDefaultWidth,
                       kVgaDefaultHeight};
  SetRect(rect_ptr, 0);
  SDL_UpdateTexture(vga_texture, nullptr, pixels_,
                    vga_screen_width_ * sizeof(uint32_t));
  SDL_RenderCopy(rend, vga_texture, nullptr, rect_ptr);

  is_r_len8_ = pin_array[VGA_R0].vector_len == 8;
  is_g_len8_ = pin_array[VGA_G0].vector_len == 8;
  is_b_len8_ = pin_array[VGA_B0].vector_len == 8;
  is_all_len8_ = is_r_len8_ && is_g_len8_ && is_b_len8_;
  if (is_r_len8_) p_r_ = static_cast<uint8_t *>(pin_array[VGA_R0].ptr);
  if (is_g_len8_) p_g_ = static_cast<uint8_t *>(pin_array[VGA_G0].ptr);
  if (is_b_len8_) p_b_ = static_cast<uint8_t *>(pin_array[VGA_B0].ptr);
  int vga_blank_n_len = pin_array[VGA_BLANK_N].vector_len;
  ABSL_CHECK(vga_blank_n_len == 1 || vga_blank_n_len == 0);
  vga_blank_n_ptr = static_cast<uint8_t *>(pin_array[VGA_BLANK_N].ptr);
  p_pixel_ = pixels_;
  p_pixel_end_ = pixels_ + vga_screen_width_ * vga_screen_height_;
}

Vga::~Vga() {
  SDL_DestroyTexture(GetTexture(0));
  delete[] pixels_;
}

void Vga::UpdateGui() {
  SDL_Texture *vga_texture = GetTexture(0);
  SDL_UpdateTexture(vga_texture, nullptr, pixels_,
                    vga_screen_width_ * sizeof(uint32_t));
  SDL_RenderCopy(GetRenderer(), vga_texture, nullptr, GetRect(0));
  SetRedraw();
}

uint32_t Vga::GetPixelColorSlowpath() {
#define NVBOARD_CONCAT3(a, b, c) NVBOARD_CONCAT(NVBOARD_CONCAT(a, b), c)
#define NVBOARD_MAP2(c, f, x) c(f, x)
#define GET_COLOR_BIT(color, n) \
  (PinPeek(NVBOARD_CONCAT3(VGA_, color, n)) << n)
#define BITS(f, color)                                                         \
  f(color, 0) f(color, 1) f(color, 2) f(color, 3) f(color, 4) f(color, 5)    \
      f(color, 6) f(color, 7)
#define GET_COLOR_BIT_REDUCE(color, n) GET_COLOR_BIT(color, n) |
#define GET_COLOR(color) NVBOARD_MAP2(BITS, GET_COLOR_BIT_REDUCE, color) 0
  int r = is_r_len8_ ? *p_r_ : GET_COLOR(R);
  int g = is_g_len8_ ? *p_g_ : GET_COLOR(G);
  int b = is_b_len8_ ? *p_b_ : GET_COLOR(B);
#undef GET_COLOR
#undef GET_COLOR_BIT_REDUCE
#undef BITS
#undef GET_COLOR_BIT
#undef NVBOARD_MAP2
#undef NVBOARD_CONCAT3
  uint32_t color = (r << 16) | (g << 8) | b;
  return color;
}

__attribute__((noinline)) void Vga::FinishOneFrame() {
  p_pixel_ = pixels_;
  if (!is_pixels_same_) {
    UpdateGui();
    is_pixels_same_ = true;
  }
}

void Vga::UpdateState() {
  if (ABSL_PREDICT_FALSE(vga_clk_cycle_minus_1 > 0)) {
    if (vga_clk_cnt_ > 0) {
      vga_clk_cnt_--;
      return;
    }
    vga_clk_cnt_ = vga_clk_cycle_minus_1;
  }

  uint32_t color = 0;
  if (ABSL_PREDICT_TRUE(is_all_len8_))
    color = ((*p_r_) << 16) | ((*p_g_) << 8) | (*p_b_);
  else
    color = GetPixelColorSlowpath();
  bool is_same = (*p_pixel_ == color);
  is_pixels_same_ &= is_same;
  *p_pixel_ = color;
  p_pixel_++;
  if (ABSL_PREDICT_FALSE(p_pixel_ == p_pixel_end_)) {
    FinishOneFrame();
  }
}

void Vga::SetExternalFramebuffer(uint32_t *fb, int w, int h) {
  ext_fb_ = fb;
  ext_fb_w_ = w;
  ext_fb_h_ = h;
}

void Vga::SyncFromFramebuffer() {
  if (!ext_fb_) return;
  int copy_w = ext_fb_w_ < vga_screen_width_ ? ext_fb_w_ : vga_screen_width_;
  int copy_h =
      ext_fb_h_ < vga_screen_height_ ? ext_fb_h_ : vga_screen_height_;
  for (int y = 0; y < copy_h; y++) {
    std::memcpy(pixels_ + y * vga_screen_width_, ext_fb_ + y * ext_fb_w_,
                copy_w * sizeof(uint32_t));
  }
  UpdateGui();
}

void VgaSetClkCycle(int cycle) { vga_clk_cycle_minus_1 = cycle - 1; }

Vga *&GetVgaInstance() { return vga_instance; }

namespace {

void InitRenderLocal(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0);
  SDL_Point p[3];
  p[0] = MakePoint(0, kWindowHeight / 2) + MakePoint(30, 0) -
         MakePoint(0, kChHeight);
  p[1] = p[0] - MakePoint(16, 0);
  p[2] = MakePoint(p[1].x, kWindowHeight / 2);
  DrawThickerLine(renderer, p, 3);

  DrawStr(renderer, "VGA", p[0].x + 4, p[0].y - kChHeight / 2, 0xffffff);
}

}  // namespace

void InitVga(SDL_Renderer *renderer) {
  InitRenderLocal(renderer);
  vga = new Vga(renderer, 1, 0, ComponentType::kVga);
  for (int p = VGA_VSYNC; p <= VGA_B7; p++) {
    vga->AddPin(p);
  }
}

void VgaUpdate() { vga->UpdateState(); }

}  // namespace nvboard
