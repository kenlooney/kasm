org 0x7C00;

mov ax, 0x1111;

clc;
jc unexpected_carry;
jnc clear_path;
mov ax, 0xDEAD;

clear_path:
stc;
jnc unexpected_clear;
jc success;
mov ax, 0xBEEF;

success:
hlt;

unexpected_carry:
mov ax, 0xAAAA;
hlt;

unexpected_clear:
mov ax, 0xBBBB;
hlt;
