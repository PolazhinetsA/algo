## Huffman encoder/decoder

compile: `$ gcc -O3 -o test test.c huffman.c`

`-O3` is necessary, as otherwise `bitio.c` would be required (linkable versions of inline functions)

usage:

`$ ./test e <srcfile >dstfile`

`$ ./test d <srcfile >dstfile`
