[BITS 32]               ; 32-bit mode

[EXTERN kernel_main]   ; External function defined elsewhere

GLOBAL _start          ; Entry point
GLOBAL reboot          ; Function to reboot the machine
GLOBAL shutdown        ; Function to shutdown the machine

SECTION .text

_start:
    call kernel_main   ; Call the main kernel function
    jmp $              ; Infinite loop to prevent falling through

reboot:
    mov al, 0xFE       ; Command to reboot
    out 0x64, al       ; Send command to keyboard controller
    hlt                ; Halt CPU
    jmp reboot         ; Loop to ensure reboot command

shutdown:
    mov al, 0xF4       ; Command to shutdown (QEMU specific)
    out 0xB004, al     ; Send shutdown signal
    hlt                ; Halt CPU
    jmp shutdown       ; Loop to ensure shutdown

