org 0x7C00;

cli;
hang:
hlt;
jmp8 hang;

padto 510, 0;
dw 0xAA55;
