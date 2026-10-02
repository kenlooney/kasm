org 0x7c00;

start:
    mov ax, 0;
    mov ds, ax;
    mov es, ax;

halt:
    hlt;
    jmp8 halt;

padto 510, 0;
dw 0xAA55;
