#ifndef NVBOARD_NVBOARD_H_
#define NVBOARD_NVBOARD_H_

#include <cstdint>

#include "nvboard/pins.h"

namespace nvboard {

void Init(int vga_clk_cycle = 1);
void Quit();
void BindPin(void *signal, int len, ...);
void Update();

// Virtual device API (for use without pin bindings, e.g. spike+nvboard)
bool KbdAvailable();
uint8_t KbdDequeue();

void VgaSetFramebuffer(uint32_t *fb, int w, int h);
void VgaSync();

void UartPutchar(uint8_t ch);
bool UartAvailable();
uint8_t UartGetchar();

uint16_t SwRead();
uint8_t BtnRead();
void LedWrite(uint16_t val);
void Seg7Write(int idx, uint8_t segments);

}  // namespace nvboard

#endif  // NVBOARD_NVBOARD_H_
