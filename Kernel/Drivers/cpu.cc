#include "cpu.h"

extern "C" void halt() {
    __asm__ volatile("hlt");
}