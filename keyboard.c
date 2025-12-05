//
// Created by diyorbek on 12/4/25.
//
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


char backspace(size_t command_index, size_t row, size_t column) {
    if (command_index == 0 && row == 0 && column == 0) {
        return -1;
    }
    return 'y';
}
