#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *help =
"Display binary file of known format in readable form.\n"
"\nusage: ./rb <filename> <formatstring>\n"
"\nGRAMMAR\n"
"\nitem specifiers\n"
"   c: read one byte and print as character\n"
"   b/B, w/W, d/D, q/Q:\n"
"      read a byte/word/dword/qword (1/2/4/8 bytes) and print as integer\n"
"      lowercase is for signed, uppercase is for unsigned\n"
"   f: float (32 bits)\n"
"   F: double (64 bits)\n"
"   n: newline\n"
"   'text': just print text in single quotes\n"
"\nflags are cancelled by lowercase equivalent\n"
"   A: read items as aligned by its size boundary\n"
"      current position is taken for base address\n"
"   S: silence the processing (don't print)\n"
"   Z: always 'S' when zero occurs\n"
"      e.g. Zcccccccccccccccczs reads out 16-byte array\n"
"      but only prints null-terminated string from it\n"
"\n[tlN ...] :\n"
"   repeatedly process ...\n"
"   't' and 'l' are optional\n"
"   t: terminate on value X\n"
"   otherwise just repeat X times\n"
"   l: X is Nth last processed value\n"
"      0th is undefined unless there was another loop right before\n"
"      in which case number of previous loop iterations is taken\n"
"      negative N stands for |N| minus that 0th one\n"
"   otherwise X is just N\n";

long subrou(FILE *, char *);

int main(int argc, char **argv)
{
    if (argc < 3) {
        puts(help);
        return -1;
    }

    FILE *file = fopen(argv[1], "rb");
    char *fmt = argv[2];

    subrou(file, fmt);

    return 0;
}

#define SZMEM 0x100

#define NEXTSLOT(mem, slot) \
    (slot+1==mem+SZMEM ? 0 : slot+1)

#define FMT(pref, fmt) \
    isupper(fmt) ? "%"pref"u" : "%"pref"d"

char garb[SZMEM];

#define RDPAD(file, off, bound) \
    fread(garb, 1, (unsigned )-off % bound, file)

long subrou(FILE *file, char *fmt)
{
    static char *sfmt;
    static long mem[SZMEM], *slot = mem;
    static int align = 0, silen = 0, zsilen = 0, curoff;
    float fval;
    double dval;

    sfmt = fmt;

    for (; *fmt && *fmt != ']'; ++fmt) {
        switch (*fmt) {
        case 'B': case 'b': case 'c':
            ++curoff;
            *slot = fgetc(file);
            if (zsilen && !*slot) silen = 1;
            if (!silen) *fmt == 'c' ? putchar(*slot)
                                    : printf(FMT("hh", *fmt), (char )*slot);
            slot = NEXTSLOT(mem, slot);
            break;
        case 'W': case 'w':
            if (align) curoff += RDPAD(file, curoff, sizeof(short));
            curoff += fread((*slot = 0, slot), 1, sizeof(short), file);
            if (zsilen && !*slot) silen = 1;
            if (!silen) printf(FMT("h", *fmt), (short )*slot);
            slot = NEXTSLOT(mem, slot);
            break;
        case 'D': case 'd':
            if (align) curoff += RDPAD(file, curoff, sizeof(int));
            curoff += fread((*slot = 0, slot), 1, sizeof(int), file);
            if (zsilen && !*slot) silen = 1;
            if (!silen) printf(FMT("", *fmt), (int )*slot);
            slot = NEXTSLOT(mem, slot);
            break;
        case 'Q': case 'q':
            if (align) curoff += RDPAD(file, curoff, sizeof(long));
            curoff += fread((*slot = 0, slot), 1, sizeof(long), file);
            if (zsilen && !*slot) silen = 1;
            if (!silen) printf(FMT("l", *fmt), (long )*slot);
            slot = NEXTSLOT(mem, slot);
            break;
        case 'F':
            if (align) curoff += RDPAD(file, curoff, sizeof(double));
            curoff += fread(&dval, 1, sizeof(double), file);
            if (zsilen && dval == 0.0) silen = 1;
            if (!silen) printf("%lf", dval);
            *slot = (long )dval;
            slot = NEXTSLOT(mem, slot);
            break;
        case 'f':
            if (align) curoff += RDPAD(file, curoff, sizeof(float));
            curoff += fread(&fval, 1, sizeof(float), file);
            if (zsilen && fval == 0.0) silen = 1;
            if (!silen) printf("%f", fval);
            *slot = (long )fval;
            slot = NEXTSLOT(mem, slot);
            break;
        case 'n':
            putchar('\n');
            break;
        case '\'':
            while (*++fmt != '\'') putchar(*fmt);
            break;
        case 'A': case 'a':
            align = isupper(*fmt);
            curoff = 0;
            break;
        case 'S': case 's':
            silen = isupper(*fmt);
            break;
        case 'Z': case 'z':
            zsilen = isupper(*fmt);
            break;
        case '[': {
            ++fmt;
            int term, last, x, n = 0;
            if (term = *fmt == 't') ++fmt;
            if (last = *fmt == 'l') ++fmt;
            sscanf(fmt, "%d", &x);
            fmt = strchr(fmt, ' ') + 1;

            if (last) x = x < 0 ? -x - *slot
                                : slot - x < mem ? slot[SZMEM-x]
                                                 : slot[-x];

            if (term) while (++n, x != subrou(file, fmt));
                 else while (++n, x--) subrou(file, fmt);

            fmt = sfmt;

            *slot = n;
            break;
        }
        default:
            putchar(*fmt);
        }
    }
    sfmt = fmt;

    return slot==mem ? mem[SZMEM-1] : slot[-1];
}
