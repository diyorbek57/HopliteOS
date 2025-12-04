#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "io.h"

#define VGA_WIDTH 80

void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end){
outb(0x3D4, 0x0A);
outb(0x3D5, cursor_start);

    outb(0x3D4, 0x0b);
    outb(0x3D5, cursor_end);

}

void vga_update_cursor(int row, int column){
uint16_t pos = row * VGA_WIDTH + column;


    outb(0x3D4, 0x0E);
    outb(0x3D5, (pos >> 8) & 0xFF);

    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);

}