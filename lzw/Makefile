all: lzw lzw-O3

lzw: lzw.c bitio.h bitio.c htbl.h htbl.c
	gcc -o lzw lzw.c bitio.c htbl.c

lzw-O3: lzw.c bitio.h bitio.c htbl.h htbl.c
	gcc -O3 -o lzw-O3 lzw.c bitio.c htbl.c
