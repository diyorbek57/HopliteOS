
#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void vga_update_cursor(int row, int col);

#endif
