#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 0x400

void add(char *dst, const char *src1, const char *src2);
void sub(char *dst, const char *src1, const char *src2);
void mul(char *dst, const char *src1, const char *src2);
void dvd(char *dst, const char *src1, const char *src2);
void pwr(char *dst, int base, int p);

int main(int argc, char **argv)
{
    char res[MAXLEN];

    switch (argv[2][0]) {
    case '+': add(res, argv[1], argv[3]); break;
    case '-': sub(res, argv[1], argv[3]); break;
    case 'x': mul(res, argv[1], argv[3]); break;
    case '/': dvd(res, argv[1], argv[3]); break;
    case '^': pwr(res, atoi(argv[1]), atoi(argv[3])); break;
    }
    puts(res);
}

enum {Add, Sub, Mul, Div};

void arith(int op, char *dst, const char *src1, const char *src2);

void add_(int *dst, const int *src1, const int *src2);
void sub_(int *dst, const int *src1, const int *src2);
void mul_(int *dst, const int *src1, const int *src2);
void dvd_(int *dst, const int *src1, const int *src2);
void pwr_(int *dst, int base, int p);

void bigcpy(int *dst, const int *src);
int bigcmp(const int *big1, const int *big2);

void str2big(int *dst, const char *src);
void big2str(char *dst, const int *src);

void add(char *dst, const char *src1, const char *src2)
{
    arith(Add, dst, src1, src2);
}

void sub(char *dst, const char *src1, const char *src2)
{
    arith(Sub, dst, src1, src2);
}

void mul(char *dst, const char *src1, const char *src2)
{
    arith(Mul, dst, src1, src2);
}

void dvd(char *dst, const char *src1, const char *src2)
{
    arith(Div, dst, src1, src2);
}

void pwr(char *dst, int base, int p)
{
    int _dst[MAXLEN/8];

    pwr_(_dst, base, p);

    big2str(dst, _dst);
}

void (*func[])(int *, const int *, const int *) = { add_,
                                                    sub_,
                                                    mul_,
                                                    dvd_ };

void arith(int op, char *dst, const char *src1, const char *src2)
{
    int sign = 0, inv = 0;

    if (op == Mul || op == Div) {
        if (*src1 == '-' && ++src1) sign = !sign;
        if (*src2 == '-' && ++src2) sign = !sign;
    } else {
        if (*src1 == '-' && ++src1) op = !op, sign = !sign;
        if (*src2 == '-' && ++src2) op = !op;
    }

    while (*src1 == '0') ++src1;
    while (*src2 == '0') ++src2;

    int buf0[MAXLEN/8],
        buf1[MAXLEN/8],
        buf2[MAXLEN/8],
        *_dst  = buf0,
        *_src1 = buf1,
        *_src2 = buf2,
        *_tmp;

    str2big(_src1, src1);
    str2big(_src2, src2);

    if (op == Sub || op == Div) {
        if (bigcmp(_src1, _src2) < 0) {
            _tmp  = _src1;
            _src1 = _src2;
            _src2 = _tmp;
            op == Sub ? (sign = !sign) : (inv = !inv);
        }
    }

    func[op](_dst, _src1, _src2);

    if (sign) *dst++ = '-';
    if (inv) *dst++ = '1', *dst++ = '/';
    big2str(dst, _dst);
}

void add_(int *dst, const int *src1, const int *src2)
{
    int r, car, s1, s2, v;

    for (r = 0, car = 0, s1 = 1, s2 = 1; s1 || s2; ++r)
    {
        if (s1 && src1[r] == -1) s1 = 0;
        if (s2 && src2[r] == -1) s2 = 0;

        v = (s1 ? src1[r] : 0) + (s2 ? src2[r] : 0) + car;
        dst[r] = v % 1000000000;
        car = v / 1000000000;
    }
    while (r-- && !dst[r]);
    dst[r+1] = -1;
}

void sub_(int *dst, const int *src1, const int *src2)
{
    int r, car, s1, s2, v;

    for (r = 0, car = 0, s1 = 1, s2 = 1; s1 || s2; ++r)
    {
        if (s1 && src1[r] == -1) s1 = 0;
        if (s2 && src2[r] == -1) s2 = 0;

        v = (s1 ? src1[r] : 0) - (s2 ? src2[r] : 0) - car;
        dst[r] = (1000000000 + v) % 1000000000;
        car = v < 0;
    }
    while (r-- && !dst[r]);
    dst[r+1] = -1;
}

void mul_(int *dst, const int *src1, const int *src2)
{
    int _dst[MAXLEN/8],
        _src[MAXLEN/8];
    long long r1, r2, car, v;

    _dst[0] = -1;

    for (r1 = 0; src1[r1] != -1; ++r1) {
        memset(_src, 0, r1 * sizeof(int));
        for (r2 = 0, car = 0; src2[r2] != -1; ++r2) {
            v = (long long )src1[r1] * (long long )src2[r2] + car;
            _src[r1+r2] = v % 1000000000;
            car = v / 1000000000;
        }
        if (car) _src[r1+r2++] = car;
        _src[r1+r2] = -1;

        add_(_dst, _dst, _src);
    }
    bigcpy(dst, _dst);
}

void dvd_(int *dst, const int *src1, const int *src2)
{
    dst[0] = -1;

    int one[] = {1, -1},
        two[] = {2, -1},
        fac[MAXLEN/8],
        rem[MAXLEN/8],
        pre[MAXLEN/8],
        acc[MAXLEN/8];

    bigcpy(rem, src1);

    while (1) 
    {
        int diff = 0;

        if (*rem == -1) break;

        bigcpy(acc, src2);
        bigcpy(fac, one);

        while (bigcmp(acc, rem) < 0) {
            mul_(fac, fac, two);
            mul_(acc, acc, two);
            ++diff;
        }
        add_(dst, dst, fac);
        sub_(rem, acc, rem);

        if (*rem == -1) break;

        bigcpy(acc, src2);
        bigcpy(fac, one);

        while (bigcmp(acc, rem) < 0) {
            mul_(fac, fac, two);
            mul_(acc, acc, two);
            --diff;
        }
        sub_(dst, dst, fac);
        sub_(rem, acc, rem);

        if (!diff) break;
    }
}

void pwr_(int *dst, int base, int p)
{
    int buf[MAXLEN/8], m;

    dst[0] =  1;
    dst[1] = -1;

    buf[0] = base;
    buf[1] = -1;

    while (m = p % 2, p /= 2) {
        if (m) mul_(dst, dst, buf);
        mul_(buf, buf, buf);
    }
    mul_(dst, dst, buf);
}

void bigcpy(int *dst, const int *src)
{
    while ((*dst++ = *src++) != -1);
}

int bigcmp(const int *big1, const int *big2)
{
    const int *b1 = big1, *b2 = big2;

    while (*b1 != -1 && *b2 != -1) ++b1, ++b2;
    while (big1 < b1 && *b1 == *b2) --b1, --b2;

    return b1 < big1 ? 0
                     : *b1 < *b2 ? -1
                                 :  1;
}

void str2big(int *dst, const char *src)
{
    char buf[MAXLEN];
    strcpy(buf, src);

    char *p = buf + strlen(buf);
    while (buf <= (p -= 9)) {
        *dst++ = atoi(p);
        *p = '\0';
    }
    *dst = atoi(buf);
    dst[*dst!=0] = -1;
}

void big2str(char *dst, const int *src)
{
    char *d = dst;
    const int *s = src;

    while (*s != -1) ++s;

    if (s != src) {
        sprintf(d, "%d", *--s);
        d += strlen(d);
        while (src < s) {
            sprintf(d, "%09d", *--s);
            d += 9;
        }
    } else strcpy(dst, "0");
}
