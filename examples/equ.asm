org 0x7C00;
start:
db 0;
end:

span equ end - start;
start_address equ start;

dw span;
dw start_address;