#ifndef BITIO_H
#define BITIO_H

#include <stdio.h>

void *bopen  (FILE *file);
int   bget   (void *this, unsigned *dst, unsigned len);
void  bput   (void *this, unsigned  src, unsigned len);
void  bflush (void *this);

#endif
