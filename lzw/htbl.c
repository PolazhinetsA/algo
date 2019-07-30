#include "htbl.h"

typedef struct chain {
    void *item;
    struct chain *next;
} chain_t;

#define chain_add(this, item) {                 \
    chain_t *__this = malloc(sizeof(chain_t));  \
    __this->item = (item);                      \
    __this->next = (this);                      \
    (this) = __this;                            \
}

#define chain_del(this) {               \
    chain_t *__this = (this);           \
    (this) = ((chain_t *)(this))->next; \
    free(__this);                       \
}

#define chain_free(this) { while (this) chain_del(this); }

typedef struct htbl {
    chain_t **chain;
    int       sz;
    int     (*hash)(void *, int);
    int     (*cmp)(void *, void *);
} htbl_t;

void *htbl_new(int sz, int (*hash)(void *, int),
                           int (*cmp)(void *, void *))
{
    htbl_t *this = calloc(1, sizeof(htbl_t));

    this->chain = calloc(sz, sizeof(chain_t *));
    this->sz = sz;
    this->hash = hash;
    this->cmp = cmp;

    return this;
}

void *htbl_free(void *_this, void (*item_free)(void *))
{
    htbl_t *this = _this;

    for (int i = 0; i < this->sz; ++i) {
        if (item_free) {
            for (chain_t *pitem = this->chain[i]; pitem; pitem = pitem->next)
                item_free(pitem->item);
        }
        chain_free(this->chain[i]);
    }
    free(this->chain);
    free(this);
}

void ***htbl_find_(void *_this, void *item)
{
    htbl_t *this = _this;

    int idx = this->hash(item, this->sz);
    chain_t **ret = &this->chain[idx];
    while(*ret && this->cmp((*ret)->item, item)) {
        ret = &(*ret)->next;
    }
    return ret;
}

void *htbl_find(void *_this, void *item)
{
    void ***pitem = htbl_find_(_this, item);
    return *pitem ? **pitem : NULL;
}

void htbl_add(void *_this, void *item)
{
    if (htbl_find(_this, item)) return;

    htbl_t *this = _this;

    int idx = this->hash(item, this->sz);
    chain_add(this->chain[idx], item);
}

void htbl_del(void *_this, void *item)
{
    void ***ppitem = htbl_find_(_this, item);
    if (*ppitem) chain_del(*ppitem);
}

void htbl_print(void *_this, int (*print)(void *, FILE *), FILE *file)
{
    htbl_t *this = _this;

    for (int i = 0; i < this->sz; ++i) {
        for (chain_t *pitem = this->chain[i]; pitem; pitem = pitem->next) {
            fputs("-[", file);
            print(pitem->item, file);
            fputc(']', file);
        }
        fputc('\n', file);
    }
}

int memhash(const char *ptr, int len, int rng)
{
    int ret = 0;
    for (int rem = 1; len--; ++ptr) {
        ret = (ret + rem * (unsigned char )*ptr) % rng;
        rem = rem * 0x100 % rng;
    }
    return ret;
}

int strhash(const char *str, int rng)
{
    return memhash(str, strlen(str), rng);
}
