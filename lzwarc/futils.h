#ifndef FUTILS_H
#define FUTILS_H

#include <stdio.h>
#include <string.h>

#ifdef __unix__

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

FILE *fopen_mkdir(char *path, char *mode);

#else

#define fopen_mkdir fopen

#endif

size_t fcopy(FILE *dst, FILE *src, size_t nbytes);
void fputs0(char *str, FILE *file);
void fgets0(char *str, FILE *file);

#endif
