#include <stdlib.h>

enum { Tchain, Toaddr, Tfixed, Ntypes };

void *hashtabl_new(int type, int maxn, int (keyf*)(void *), int maxk);
void *hashtabl_find(void *this, int key);
void hashtabl_insert(void *this, void *item);
