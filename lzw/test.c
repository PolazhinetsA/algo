#include "lzw.h"

int main(int argc, char **argv)
{
    if (argc != 2 || strcmp(argv[1], "enc") && strcmp(argv[1], "dec")) {
        fputs("usage:\n"
              "./test enc <infile >outfile\n"
              "./test dec <infile >outfile\n", stderr);
        return 1;
    }
    switch (argv[1][0]) {
        case 'e': encode(stdin, stdout); break;
        case 'd': decode(stdin, stdout); break;
    }
}
