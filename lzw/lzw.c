#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitio.h"
#include "htbl.h"

void encode(FILE *, FILE *);
void decode(FILE *, FILE *);

int main(int argc, char **argv)
{
    if (argc != 2 || strcmp(argv[1], "enc") && strcmp(argv[1], "dec")) {
        fputs("usage:\n"
              "./lzw enc <infile >outfile\n"
              "./lzw dec <infile >outfile\n", stderr);
        return 1;
    }
    switch (argv[1][0]) {
        case 'e': encode(stdin, stdout); break;
        case 'd': decode(stdin, stdout); break;
    }
}

typedef struct {
    char *ptr;
    int   len;
} dent_t;

typedef struct {
    dent_t *ent;
    int     nent;
    void   *htbl;
} dict_t;

dict_t *dict_new();
dict_t *dict_free(dict_t *);
void dict_add(dict_t *, char *, int);
int dict_find(dict_t *, char *, int);

void encode(FILE *in, FILE *out)
{
    dict_t *dict = dict_new();
    void *bout = bopen(out);

    for (char buf[0x1000]; fread(buf, 1, 1, in); ) {
        int len = 1;
        do buf[len++] = fgetc(in); while (-1 != dict_find(dict, buf, len));
        ungetc(buf[--len], in);
        bput(bout, dict_find(dict, buf, len), 12);
        dict_add(dict, buf, len+1);
    }
    bflush(bout);

    free(bout);
    dict_free(dict);
}

void decode(FILE *in, FILE *out)
{
    dict_t *dict = dict_new();
    void *bin = bopen(in);

    for (int idx_ = -1, _idx; -1 != bget(bin, &_idx, 12); idx_ = _idx)
    {
        dent_t *pent_ = &dict->ent[idx_],
               *_pent = &dict->ent[_idx];

        if (idx_ != -1 && dict->nent < 0x1000) {
            char buf[0x1000];
            memcpy(buf, pent_->ptr, pent_->len);
            buf[pent_->len] = _pent->ptr ? *_pent->ptr : *pent_->ptr;
            dict_add(dict, buf, pent_->len+1);
        }
        fwrite(_pent->ptr, 1, _pent->len, out);
    }

    free(bin);
    dict_free(dict);
}


int enthash(dent_t *, int);
int entcmp(dent_t *, dent_t *);

dict_t *dict_new()
{
    dict_t *this = malloc(sizeof(dict_t));
    this->ent = calloc(0x1000, sizeof(dent_t));
    this->nent = 0;
    this->htbl = htbl_new(2003, enthash, entcmp);

    char ch = '\0';
    do dict_add(this, &ch, 1); while (++ch);

    return this;
}

dict_t *dict_free(dict_t *this)
{
    for (int i = 0; i < 0x1000; ++i) {
        if (!this->ent[i].ptr) break;
        free(this->ent[i].ptr);
    }
    htbl_free(this->htbl, NULL);
    free(this->ent);
    free(this);
}

void dict_add(dict_t *this, char *ptr, int len)
{
    if (this->nent == 0x1000) return;

    memcpy(this->ent[this->nent].ptr = malloc(len), ptr,
           this->ent[this->nent].len = len);
    htbl_add(this->htbl, &this->ent[this->nent++]);
}

int dict_find(dict_t *this, char *ptr, int len)
{
    dent_t ent;
    ent.ptr = ptr;
    ent.len = len;

    dent_t *pent = htbl_find(this->htbl, &ent);
    return pent ? pent - this->ent : -1;;
}

int enthash(dent_t *ent, int rng)
{
    return memhash(ent->ptr, ent->len, rng);
}

int entcmp(dent_t *ent1, dent_t *ent2)
{
    int dl = ent1->len - ent2->len,
        ret = memcmp(ent1->ptr, ent2->ptr, dl < 0 ? ent1->len : ent2->len);

    return ret ? ret : dl;
}
