#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "diter.h"
#include "futils.h"
#include "lzw.h"

#define USAGE {                                         \
    fputs("usage:\n"                                    \
          "./lzwarc a archivename item1 item2 ...\n"    \
          "./lzwarc x archivename [dst_dir_path/]\n"    \
          "./lzwarc l archivename\n",                   \
          stderr);                                      \
    return 1;                                           \
}

void archive(char **ppath);
void extract(char **ppath);
void lstcont(char **ppath);

int main(int argc, char **argv)
{
    if (argc < 3) USAGE;

    switch (*argv[1]) {
        case 'a':
            if (argc < 4) USAGE;
            archive(argv+2);
            break;
        case 'x':
            extract(argv+2);
            break;
        case 'l':
            lstcont(argv+2);
            break;
        default:
            USAGE;
    }
}

void archive(char **ppath)
{
    FILE *farc = fopen(*ppath++, "ab");

    for (; *ppath; ++ppath)
    {
        void *diter = dopen(*ppath);

        for (char fpath[PATH_MAX]; dnext(diter, fpath); )
        {
            FILE *file, *tfile;
            uint32_t sz, sz_;

            file = fopen(fpath, "rb");
            if (!file) continue;

            tfile = tmpfile();
            lzw_encode(file, tfile);
            sz = ftell(file);
            sz_ = ftell(tfile);
            if (sz_ < sz) {
                fclose(file);
                file = tfile;
            } else {
                fclose(tfile);
                sz_ = sz;
            }
            rewind(file);

            fputs0(fpath, farc);
            fwrite(&sz, sizeof(uint32_t), 1, farc);
            fwrite(&sz_, sizeof(uint32_t), 1, farc);
            fcopy(farc, file, sz_);

            fclose(file);
        }
        dclose(diter);
    }
    fclose(farc);
}

void extract(char **ppath)
{
    FILE *farc = fopen(*ppath++, "rb");

    char dpath[PATH_MAX], *_path;
    strcpy(dpath, *ppath ? *ppath : "");
    _path = dpath + strlen(dpath);

    if (*ppath) ++ppath;

    while (fgets0(_path, farc), *_path)
    {
        uint32_t sz, sz_;
        fread(&sz, sizeof(uint32_t), 1, farc);
        fread(&sz_, sizeof(uint32_t), 1, farc);

        char **pp;
        for (pp = ppath; *pp && memcmp(_path, *pp, strlen(*pp)); ++pp);
        if (*pp || !*ppath)
        {
            FILE *file, *tfile;
            file = fopen_mkdir(dpath, "wb");
            if (sz_ < sz) {
                tfile = tmpfile();
                fcopy(tfile, farc, sz_);
                rewind(tfile);
                lzw_decode(tfile, file);
                fclose(tfile);
            } else {
                fcopy(file, farc, sz_);
            }
            fclose(file);
        }
        else
        {
            fseek(farc, sz_, SEEK_CUR);
        }
    }
    fclose(farc);
}

void lstcont(char **ppath)
{
    printf("original size | arhived size | path/filename\n"
           "--------------|--------------|--------------\n");

    FILE *farc = fopen(ppath[0], "rb");
    for (char name[0x100]; fgets0(name, farc), *name; )
    {
        uint32_t sz, sz_;
        fread(&sz, sizeof(uint32_t), 1, farc);
        fread(&sz_, sizeof(uint32_t), 1, farc);
        printf(" %12u | %12u | %s\n", sz, sz_, name);
        fseek(farc, sz_, SEEK_CUR);
    }
    fclose(farc);
}
