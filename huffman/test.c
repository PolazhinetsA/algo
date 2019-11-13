#include <stdio.h>
#include "huffman.h"

int main(int argc, char **argv)
{
    switch (argv[1][0]) {
    case 'e':
        huffman_encode(stdout, stdin);
        break;
    case 'd':
        huffman_decode(stdout, stdin);
        break;
    }
}
