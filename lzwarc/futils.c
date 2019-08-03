#include "futils.h"

#ifdef __unix__

FILE *fopen_mkdir(char *path, char *mode)
{
    for(char *p = path+1, *q;
        q = strchr(p, '/');
        p = q + 1)
    {
        *q = '\0';
        struct stat s;
        stat(path, &s);
        if (!S_ISDIR(s.st_mode))
            mkdir(path, 0777);
        *q = '/';
    }
    return fopen(path, mode);
}

#endif

size_t fcopy(FILE *dst, FILE *src, size_t nbytes)
{
    char buf[0x1000];
    size_t nread, ret;

    for(nread = sizeof(buf), ret = 0;
        nbytes >= sizeof(buf) && nread == sizeof(buf);
        nbytes -= nread, ret += nread)
    {
        nread = fread(buf, 1, sizeof(buf), src);
        fwrite(buf, 1, nread, dst);
    }
    if (nread == sizeof(buf)) {
        nread = fread(buf, 1, nbytes, src);
        fwrite(buf, 1, nread, dst);
        ret += nread;
    }

    return ret;
}

void fputs0(char *str, FILE *file)
{
    fputs(str, file);
    fputc('\0', file);
}

void fgets0(char *str, FILE *file)
{
    int ch;
    while (ch = fgetc(file), ch && ch != EOF)
        *str++ = ch;
    *str = '\0';
}
