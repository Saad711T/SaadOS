[org 7c00h]
[bits 16]

KERNEL_LOCATION equ 0x1000
BOOT_DISK: db 0

_start:
    mov [BOOT_DISK], dl
    jmp load_kernel

load_kernel:
    xor ax, ax
    mov es, ax
    mov ds, ax

    mov sp, 0x8000

    mov bx, KERNEL_LOCATION
    mov dh, 20

    mov ah, 0x02        ; BIOS interrupt to read disk sectors
    mov al, dh          ; Number of sectors to read
    mov ch, 0x00        ; Cylinder number
    mov dh, 0x00        ; Head number
    mov cl, 0x02        ; Sector number (starts from 1, so 2 = second sector)
    mov dl, [BOOT_DISK] ; Drive number
    int 0x13            ; BIOS disk service

    xor si, si
    jc disk_fail        ; Jump if carry flag is set (error)

text_mode:
    mov ah, 0x00
    mov al, 0x03        ; Set text mode 80x25
    int 0x10

gdt_init:
    cli                 ; Disable interrupts
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x01        ; Set PE bit (Protection Enable)
    mov cr0, eax

    jmp CODE_SEG:start_protected_mode

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

gdt_start:
gdt_null:
    dd 0x00
    dd 0x00

gdt_code:
    dw 0xFFFF           ; Limit
    dw 0x00             ; Base (low)
    db 0x00             ; Base (middle)
    db 10011010b        ; Access byte
    db 11001111b        ; Flags and limit (high)
    db 0x00             ; Base (high)

gdt_data:
    dw 0xFFFF
    dw 0x00
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x90000
    mov esp, ebp

    jmp KERNEL_LOCATION

disk_fail:
    mov si, msg
.next_char:
    lodsb
    or al, al
    jz .halt
    mov ah, 0x0e        ; Teletype output
    int 0x10
    jmp .next_char
.halt:
    jmp $

msg: db "code: 0x00000001 _error while reading the bootdisk !", 0

times 510 - ($ - $$) db 0x00
dw 0xAA55              ; Boot signature
