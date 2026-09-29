org 0x7C00;
std;
cld;
cmc;
clc;
nop;
cli;
lahf;
sahf;
hang:
hlt;
jmp8 hang;

padto 510, 0;
dw 0xAA55;
