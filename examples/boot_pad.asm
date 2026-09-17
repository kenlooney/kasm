mov eax, 42;
times 510-($-$$) db 0;
dw 0xAA55;
