#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "futils.h"
#include "lzw.h"

#define USAGE {                                     \
    fputs("usage:\n"                                \
          "./lzwarc -arc archive file1 file2 ...\n" \
          "./lzwarc -ext archive [dest.dir.path]\n" \
          "./lzwarc -lst archive\n",                \
          stderr);                                  \
    return 1;                                       \
}

void archive(char **ppath);
void extract(char **ppath);
void lstcont(char **ppath);

void (*routine[])(char **) = { archive, extract, lstcont };

enum { Arc, Ext, Lst };

int main(int argc, char **argv)
{
    if (argc < 3)
        USAGE;

    int job;
    char *jobstr = argv[1];
    if (!strcmp(jobstr, "-arc"))
        job = Arc;
    else if(!strcmp(jobstr, "-ext"))
        job = Ext;
    else if(!strcmp(jobstr, "-lst"))
        job = Lst;
    else
        USAGE;

    if (job == Arc && argc < 4)
        USAGE;

    routine[job](argv+2);
}

void archive(char **ppath)
{
    FILE *farc = fopen(*ppath++, "wb");
    for (; *ppath; ++ppath)
    {
        FILE *file, *tfile;
        size_t sz, sz_;

        file = fopen(*ppath, "rb");
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

        fputs0(*ppath, farc);
        fwrite(&sz, sizeof(size_t), 1, farc);
        fwrite(&sz_, sizeof(size_t), 1, farc);
        fcopy(farc, file, sz_);

        fclose(file);
    }
    fclose(farc);
}

void extract(char **ppath)
{
    char path_[0x100], *_path;
    strcpy(path_, ppath[1] ? ppath[1] : "");
    _path = path_ + strlen(path_);

    FILE *farc = fopen(ppath[0], "rb");
    while (fgets0(_path, farc), *_path)
    {
        FILE *file, *tfile;
        size_t sz, sz_;

        fread(&sz, sizeof(size_t), 1, farc);
        fread(&sz_, sizeof(size_t), 1, farc);
        file = fopen_mkdir(path_, "wb");
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
    fclose(farc);
}

void lstcont(char **ppath)
{
    printf("original size | arhived size | path/filename\n"
           "--------------|--------------|--------------\n");

    FILE *farc = fopen(ppath[0], "rb");
    for (char name[0x100]; fgets0(name, farc), *name; )
    {
        size_t sz, sz_;
        fread(&sz, sizeof(size_t), 1, farc);
        fread(&sz_, sizeof(size_t), 1, farc);
        printf(" %12lu | %12lu | %s\n", sz, sz_, name);
        fseek(farc, sz_, SEEK_CUR);
    }
    fclose(farc);
}
