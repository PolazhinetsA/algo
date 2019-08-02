#ifndef LZW_H
#define LZW_H

#include <stdio.h>
#include <stdlib.h>

void encode(FILE *in, FILE *out);
void decode(FILE *in, FILE *out);

#endif
