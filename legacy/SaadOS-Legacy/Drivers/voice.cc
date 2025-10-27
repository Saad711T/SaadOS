#include "voice.h"
#include "ports.h"

void beep() {
    outb(0x61, inb(0x61) | 3);
}
