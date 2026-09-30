inc ax;
inc cx;
inc dx;
inc bx;
inc sp;
inc bp;
inc si;
inc di;

dec ax;
dec cx;
dec dx;
dec bx;
dec sp;
dec bp;
dec si;
dec di;

push ax;
push cx;
push dx;
push bx;
push sp;
push bp;
push si;
push di;
pop di;
pop si;
pop bp;
pop sp;
pop bx;
pop dx;
pop cx;
pop ax;

mov ax, 1;
mov di, 0x1234;

xchg ax, ax;
xchg ax, cx;
xchg ax, dx;
xchg ax, bx;
xchg ax, sp;
xchg ax, bp;
xchg ax, si;
xchg ax, di;

stop:
hlt;
jmp8 stop;
