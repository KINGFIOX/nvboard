#ifndef NVBOARD_SRC_INTERNAL_VGA_H_
#define NVBOARD_SRC_INTERNAL_VGA_H_

#include <cstdint>

#include "src/internal/component.h"

namespace nvboard {

constexpr int kVgaDefaultWidth = 640;
constexpr int kVgaDefaultHeight = 480;

enum VgaModeId {
  kVgaMode640x480,
  kNrVgaMode,
};

struct VgaMode {
  int h_frontporch, h_active, h_backporch, h_total;
  int v_frontporch, v_active, v_backporch, v_total;
};

class Vga : public Component {
 public:
  Vga(SDL_Renderer *rend, int cnt, int init_val, ComponentType ct);
  ~Vga() override;

  void UpdateGui() override;
  void UpdateState() override;

  void SetExternalFramebuffer(uint32_t *fb, int w, int h);
  void SyncFromFramebuffer();

 private:
  uint32_t GetPixelColorSlowpath();
  void FinishOneFrame();

  int vga_screen_width_;
  int vga_screen_height_;
  uint32_t *pixels_;
  int vga_clk_cnt_;
  uint32_t *p_pixel_;
  uint32_t *p_pixel_end_;
  uint8_t *p_r_;
  uint8_t *p_g_;
  uint8_t *p_b_;
  bool is_r_len8_;
  bool is_g_len8_;
  bool is_b_len8_;
  bool is_all_len8_;
  bool is_pixels_same_;

  uint32_t *ext_fb_;
  int ext_fb_w_;
  int ext_fb_h_;
};

}  // namespace nvboard

#endif  // NVBOARD_SRC_INTERNAL_VGA_H_
