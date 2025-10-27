#include "input.h"
#include "ports.h"

void init_keyboard() {
        outb(0x64, 0xAE);
        
        outb(0x60, 0xF4);

    }
    

char get_key() {
    uint8_t scancode = inb(0x60);
    return (char)scancode;
}

void init_mouse() {
}

void handle_mouse() {
}
