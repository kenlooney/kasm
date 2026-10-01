org 0x7C00;
jmpfar 0x0000, normalized;
normalized:
cli;
mov ax, 0;
mov ds, ax;
mov es, ax;
mov ss, ax;
mov sp, 0x7C00;
sti;
hang:
hlt;
jmp8 hang;
padto 510, 0;
dw 0xAA55;
