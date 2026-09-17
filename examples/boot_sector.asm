mov ax, 42;
add ax, 3;
times (512-0b00000010)-($-$$) db 0;
dw 0xAA55;