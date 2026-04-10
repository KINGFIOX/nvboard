#ifndef __NVBOARD_H__
#define __NVBOARD_H__

#include <pins.h>
#include <stdint.h>

void nvboard_init(int vga_clk_cycle = 1);
void nvboard_quit();
void nvboard_bind_pin(void *signal, int len, ...);
void nvboard_update();

// Virtual device API (for use without pin bindings, e.g. spike+nvboard)
bool nvboard_kbd_available();
uint8_t nvboard_kbd_dequeue();

void nvboard_vga_set_framebuffer(uint32_t *fb, int w, int h);
void nvboard_vga_sync();

void nvboard_uart_putchar(uint8_t ch);
bool nvboard_uart_available();
uint8_t nvboard_uart_getchar();

uint16_t nvboard_sw_read();
uint8_t nvboard_btn_read();
void nvboard_led_write(uint16_t val);
void nvboard_seg7_write(int idx, uint8_t segments);

#endif
