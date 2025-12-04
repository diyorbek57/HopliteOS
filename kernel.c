
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000
#define COMMAND_BUFFER_SIZE 256
#define MEMORY_POOL_SIZE 32768  // 32KB memory pool
#define MAX_FILES 16
#define MAX_FILENAME 32
#define MAX_FILE_SIZE 1024

#define CURSOR_HIDE "\033[?25l"
#define CURSOR_SHOW "\033[?25h"
#define CLEAR_LINE "\r"

enum vga_color {
    VGA_BLACK = 0, VGA_BLUE = 1, VGA_GREEN = 2, VGA_CYAN = 3,
    VGA_RED = 4, VGA_MAGENTA = 5, VGA_BROWN = 6, VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8, VGA_LIGHT_BLUE = 9, VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11, VGA_LIGHT_RED = 12, VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW = 14, VGA_WHITE = 15,
};

// ============= STRING FUNCTIONS (Must be first!) =============
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void int_to_str(int num, char* str) {
    int i = 0;
    bool negative = false;

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    if (num < 0) {
        negative = true;
        num = -num;
    }

    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }

    if (negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse string
    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

// ============= TIMER SYSTEM =============
static volatile uint32_t system_ticks = 0;

void timer_init(void) {
    system_ticks = 0;
}

void timer_tick(void) {
    system_ticks++;
}

uint32_t get_uptime_seconds(void) {
    return system_ticks / 18;  // Rough estimate (18.2 ticks per second)
}

// ============= MEMORY MANAGEMENT =============
typedef struct {
    void* addr;
    size_t size;
    bool used;
} MemBlock;

static uint8_t memory_pool[MEMORY_POOL_SIZE];
static MemBlock mem_blocks[64];
static size_t mem_blocks_count = 0;
static size_t total_allocated = 0;

void memory_init(void) {
    mem_blocks_count = 0;
    total_allocated = 0;

    // Initialize first free block
    mem_blocks[0].addr = memory_pool;
    mem_blocks[0].size = MEMORY_POOL_SIZE;
    mem_blocks[0].used = false;
    mem_blocks_count = 1;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    // Align to 4 bytes
    size = (size + 3) & ~3;

    // Find free block
    for (size_t i = 0; i < mem_blocks_count; i++) {
        if (!mem_blocks[i].used && mem_blocks[i].size >= size) {
            mem_blocks[i].used = true;

            // Split block if too large
            if (mem_blocks[i].size > size + 16 && mem_blocks_count < 64) {
                mem_blocks[mem_blocks_count].addr = (uint8_t*)mem_blocks[i].addr + size;
                mem_blocks[mem_blocks_count].size = mem_blocks[i].size - size;
                mem_blocks[mem_blocks_count].used = false;
                mem_blocks_count++;

                mem_blocks[i].size = size;
            }

            total_allocated += mem_blocks[i].size;
            return mem_blocks[i].addr;
        }
    }

    return NULL;  // Out of memory
}

void kfree(void* ptr) {
    if (ptr == NULL) return;

    for (size_t i = 0; i < mem_blocks_count; i++) {
        if (mem_blocks[i].addr == ptr && mem_blocks[i].used) {
            mem_blocks[i].used = false;
            total_allocated -= mem_blocks[i].size;
            return;
        }
    }
}

// ============= FILE SYSTEM =============
typedef struct {
    char name[MAX_FILENAME];
    char* content;
    size_t size;
    bool used;
} File;

static File files[MAX_FILES];

void fs_init(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = false;
        files[i].content = NULL;
    }
}

int fs_create(const char* name, const char* content) {
    // Check if file exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            return -1;  // File exists
        }
    }

    // Find free slot
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            size_t content_len = strlen(content);
            if (content_len > MAX_FILE_SIZE) return -2;  // Too large

            files[i].content = (char*)kmalloc(content_len + 1);
            if (files[i].content == NULL) return -3;  // Out of memory

            strcpy(files[i].name, name);
            strcpy(files[i].content, content);
            files[i].size = content_len;
            files[i].used = true;
            return 0;  // Success
        }
    }

    return -4;  // No free slots
}

File* fs_read(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }
    return NULL;
}

int fs_delete(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            kfree(files[i].content);
            files[i].used = false;
            return 0;
        }
    }
    return -1;  // Not found
}

// ============= VGA & TERMINAL =============
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t) c | (uint16_t) color << 8;
}

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;


void terminal_initialize(void) {

    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
        }
    }

    // Set a character at a specific position for testing
     // Place 'A' at (10, 5)

    // Draw the cursor


}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    terminal_buffer[y * VGA_WIDTH + x] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        return;
    }
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}


// Function to draw the character at the cursor's position with inverted colors

// ============= KEYBOARD =============
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static uint8_t last_scancode = 0;

char scancode_to_char(uint8_t scancode) {
    static const char scancode_map[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
        0, ' '
    };
    if (scancode < sizeof(scancode_map))
        return scancode_map[scancode];
    return 0;
}

char get_key(void) {
    uint8_t scancode = inb(0x60);
    if (scancode & 0x80) return 0;
    if (scancode == last_scancode) return 0;
    last_scancode = scancode;
    return scancode_to_char(scancode);
}

// ============= COMMANDS =============
static char command_buffer[COMMAND_BUFFER_SIZE];
static size_t command_index = 0;

void print_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    terminal_writestring("HOPLITE-OS");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("> ");
}

void cmd_help(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    terminal_writestring("\nAvailable Commands:\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("  help, clear, about, version, sysinfo\n");
    terminal_writestring("  uptime   - Show system uptime\n");
    terminal_writestring("  meminfo  - Show memory usage\n");
    terminal_writestring("  ls       - List files\n");
    terminal_writestring("  cat [f]  - Show file content\n");
    terminal_writestring("  touch [f] [content] - Create file\n");
    terminal_writestring("  rm [f]   - Delete file\n");
    terminal_writestring("  echo [t] - Echo text\n\n");
}

void cmd_uptime(void) {
    uint32_t seconds = get_uptime_seconds();
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;

    char buf[32];
    terminal_writestring("\nSystem uptime: ");

    int_to_str(hours, buf);
    terminal_writestring(buf);
    terminal_writestring("h ");

    int_to_str(minutes % 60, buf);
    terminal_writestring(buf);
    terminal_writestring("m ");

    int_to_str(seconds % 60, buf);
    terminal_writestring(buf);
    terminal_writestring("s\n\n");
}

void cmd_meminfo(void) {
    char buf[32];
    terminal_writestring("\nMemory Information:\n");
    terminal_writestring("  Total pool: ");
    int_to_str(MEMORY_POOL_SIZE, buf);
    terminal_writestring(buf);
    terminal_writestring(" bytes\n");

    terminal_writestring("  Allocated:  ");
    int_to_str(total_allocated, buf);
    terminal_writestring(buf);
    terminal_writestring(" bytes\n");

    terminal_writestring("  Free:       ");
    int_to_str(MEMORY_POOL_SIZE - total_allocated, buf);
    terminal_writestring(buf);
    terminal_writestring(" bytes\n");

    terminal_writestring("  Blocks:     ");
    int_to_str(mem_blocks_count, buf);
    terminal_writestring(buf);
    terminal_writestring("\n\n");
}

void cmd_ls(void) {
    terminal_writestring("\nFiles:\n");
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used) {
            terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
            terminal_writestring("  ");
            terminal_writestring(files[i].name);
            terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
            terminal_writestring(" (");
            char buf[16];
            int_to_str(files[i].size, buf);
            terminal_writestring(buf);
            terminal_writestring(" bytes)\n");
            count++;
        }
    }
    if (count == 0) {
        terminal_writestring("  (no files)\n");
    }
    terminal_writestring("\n");
}

void cmd_cat(const char* filename) {
    if (strlen(filename) == 0) {
        terminal_writestring("\nUsage: cat [filename]\n\n");
        return;
    }

    File* file = fs_read(filename);
    if (file == NULL) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        terminal_writestring("\nFile not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
        return;
    }

    terminal_writestring("\n");
    terminal_writestring(file->content);
    terminal_writestring("\n\n");
}

void cmd_touch(const char* args) {
    if (strlen(args) == 0) {
        terminal_writestring("\nUsage: touch [filename] [content]\n\n");
        return;
    }

    // Parse filename and content
    char filename[MAX_FILENAME];
    const char* content = args;
    int i = 0;

    while (*content && *content != ' ' && i < MAX_FILENAME - 1) {
        filename[i++] = *content++;
    }
    filename[i] = '\0';

    if (*content == ' ') content++;

    int result = fs_create(filename, content);
    if (result == 0) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
        terminal_writestring("\nFile created: ");
        terminal_writestring(filename);
        terminal_writestring("\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        terminal_writestring("\nError creating file");
        if (result == -1) terminal_writestring(" (already exists)");
        else if (result == -3) terminal_writestring(" (out of memory)");
        terminal_writestring("\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    }
}

void cmd_rm(const char* filename) {
    if (strlen(filename) == 0) {
        terminal_writestring("\nUsage: rm [filename]\n\n");
        return;
    }

    if (fs_delete(filename) == 0) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
        terminal_writestring("\nFile deleted: ");
        terminal_writestring(filename);
        terminal_writestring("\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        terminal_writestring("\nFile not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    }
}

void cmd_clear(void) {
    terminal_clear();
}

void cmd_about(void) {
    terminal_writestring("\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    terminal_writestring("================================\n");
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    terminal_writestring("      Hoplite Operating System\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    terminal_writestring("================================\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));

}

void cmd_version(void) {
    terminal_writestring("\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    terminal_writestring("Hoplite OS Version ALPHA 0.0.1\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("Built: December 2025\n\n");
}

void cmd_sysinfo(void) {
    terminal_writestring("\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    terminal_writestring("System Information:\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("  Architecture: x86 (32-bit)\n");
    terminal_writestring("  Memory Pool: 32 KB\n");
    terminal_writestring("  Max Files: 16\n");
    terminal_writestring("  Bootloader: GRUB\n\n");
}

void cmd_echo(const char* args) {
    terminal_writestring("\n");
    if (strlen(args) > 0) {
        terminal_writestring(args);
    }
    terminal_writestring("\n\n");
}

void process_command(void) {
    command_buffer[command_index] = '\0';

    if (command_index == 0) {
        terminal_writestring("\n");
        return;
    }

    char* cmd = command_buffer;
    char* args = command_buffer;

    while (*args && *args != ' ') args++;
    if (*args == ' ') {
        *args = '\0';
        args++;
    } else {
        args = "";
    }

    // Execute commands
    if (strcmp(cmd, "help") == 0) cmd_help();
    else if (strcmp(cmd, "clear") == 0) cmd_clear();
    else if (strcmp(cmd, "about") == 0) cmd_about();
    else if (strcmp(cmd, "version") == 0) cmd_version();
    else if (strcmp(cmd, "sysinfo") == 0) cmd_sysinfo();
    else if (strcmp(cmd, "uptime") == 0) cmd_uptime();
    else if (strcmp(cmd, "meminfo") == 0) cmd_meminfo();
    else if (strcmp(cmd, "ls") == 0) cmd_ls();
    else if (strcmp(cmd, "cat") == 0) cmd_cat(args);
    else if (strcmp(cmd, "touch") == 0) cmd_touch(args);
    else if (strcmp(cmd, "rm") == 0) cmd_rm(args);
    else if (strcmp(cmd, "echo") == 0) cmd_echo(args);
    else {
        terminal_writestring("\n");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        terminal_writestring("Unknown command: ");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
        terminal_writestring(cmd);
        terminal_writestring("\nType 'help' for commands.\n\n");
    }

    command_index = 0;
}

void kernel_main(void) {
    terminal_initialize();

    // Initialize systems
    timer_init();
    memory_init();
    fs_init();

    // Welcome screen
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    terminal_writestring("========================================\n");
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    terminal_writestring("     Welcome to HOPLITE OS (ALPHA) v0.0.1!\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    terminal_writestring("========================================\n\n");

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("Type 'help' to see available commands.\n\n");

    print_prompt();

    // Main loop
    while (1) {
        timer_tick();  // Increment timer

        char key = get_key();
        if (key != 0) {
            if (key == '\n') {
                terminal_putchar('\n');
                process_command();
                print_prompt();
            } else if (key == '\b') {
                if (command_index > 0) {
                    command_index--;
                    terminal_putchar('\b');
                }
            } else {
                if (command_index < COMMAND_BUFFER_SIZE - 1) {
                    command_buffer[command_index++] = key;
                    terminal_putchar(key);
                }
            }

            for (volatile int i = 0; i < 100000; i++);
        }
    }
}
