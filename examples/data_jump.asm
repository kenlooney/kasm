jmp near code;
db 65,66,0;
byte 0xAA,0x55;
code: mov eax,42;
ret;
