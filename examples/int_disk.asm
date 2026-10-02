org 0x7C00;

mov ax, 0;
mov es, ax;
mov ax, 0x0201;
mov bx, 0x7E00;
mov cx, 0x0002;
mov dx, 0x0080;
int 0x13;
jc disk_error;
jmpfar 0, second_stage;

disk_error:
mov ax, 0x0E45;
mov bx, 0x0007;
int 0x10;
cli;
hlt;

padto 510, 0;
dw 0xAA55;

second_stage:
mov ax, 0x0E42;
mov bx, 0x0007;
int 0x10;
cli;
hlt;
padto 1024, 0;
