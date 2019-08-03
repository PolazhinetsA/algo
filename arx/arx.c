#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "futils.h"

#define USAGE {                                     \
    fputs("usage:\n"                                \
          "./arx -arc archive file1 file2 ...\n"    \
          "./arx -ext archive [dest.dir.path]\n"    \
          "./arx -lst archive\n",                   \
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
    FILE *farc = fopen(*ppath++, "w");

    for (; *ppath; ++ppath)
    {
        FILE *file;
        size_t sz;

        file = fopen(*ppath, "r");
        fseek(file, 0, SEEK_END);
        sz = ftell(file);
        rewind(file);

        fputs0(*ppath, farc);
        fwrite(&sz, sizeof(size_t), 1, farc);
        fcopy(farc, file, sz);

        fclose (file);
    }

    fclose(farc);
}

void extract(char **ppath)
{
    FILE *farc = fopen(ppath[0], "r");

    char path_[0x100], *_path;
    strcpy(path_, ppath[1] ? ppath[1] : "");
    _path = path_ + strlen(path_);

    while (fgets0(_path, farc), *_path)
    {
        FILE *file;
        size_t sz;

        file = fopen_mkdir(path_, "w");
        fread(&sz, sizeof(size_t), 1, farc);
        fcopy(file, farc, sz);
        fclose(file);
    }

    fclose(farc);
}

void lstcont(char **ppath)
{
    FILE *farc = fopen(ppath[0], "r");

    for (char name[0x100]; fgets0(name, farc), *name; )
    {
        size_t sz;
        fread(&sz, sizeof(size_t), 1, farc);
        printf("%12lu bytes: %s\n", sz, name);
        fseek(farc, sz, SEEK_CUR);
    }

    fclose(farc);
}
