#include "hashtabl.h"

typedef struct {
    void **tabl;
    int type;
    int (*keyf)(void *);
    int size, maxk,
    int hf1a, hf1b,
        hf2a, hf2b;
} hashtabl_t;

#define hashfunc1(t,k) \
(((long )(k) * (t)->hf1a + (t)->hf1b) % (t)->maxk % (t)->size)

#define hashfunc2(t,k) \
(((long )(k) * (t)->hf2a + (t)->hf2b) % (t)->maxk % (t)->size)

void *hashtabl_new(int type, int maxn, int (*keyf)(void *), int maxk)
{
    static const void (*init[])(hashtabl_t *, int, int (*)(void *), int) =
      { hashtabl_init_chain,
        hashtabl_init_oaddr,
        hashtabl_init_fixed };

    if (!(0 <= type && type < Ntypes)) return NULL;

    hashtabl_t *this = malloc(sizeof(hashtabl_t));
    init[type](this, keyf, maxk);
    return this;
}

void *hashtabl_find(void *this, void *item)
{
    static const void (*find[])(hashtabl_t *, void *) =
      { hashtabl_find_chain,
        hashtabl_find_oaddr,
        hashtabl_find_fixed };

    return find[((hashtabl_t *)this)->type](this, item);
}

void hashtabl_insert(void *this, void *item)
{
    static const void (*insert[])(hashtabl_t *, void *) =
      { hashtabl_insert_chain,
        hashtabl_insert_oaddr,
        hashtabl_insert_fixed };

    insert[((hashtabl_t *)this)->type](this, item);
}

void hashtabl_init_chain(hashtabl_t *this, int maxn,
                         int (*keyf)(void *), int maxk)
{
    this->type = Tchain;
    this->keyf = keyf;
    this->size = nextprime(maxn/0x40);
    this->maxk = nextprime(maxk);
    this->hf1a = 1 + rand() % (maxk - 1);
    this->hf1b = rand() % maxk;
    this->tabl = calloc(this->size, sizeof(void *));
}

enum { Item, Next };

void ***hashtabl_search_chain(hashtabl_t *this, int key)
{
    void ***ptr = &this->tabl[hashfunc1(this,key)];

    while (*ptr && this->keyf((*ptr)[Item]) != this->keyf(item))
        ptr = &(*ptr)[Next];
}

void *hashtabl_find_chain(hashtabl_t *this, int key)
{
    return (*hashtabl_search_chain(this, key))[Item];
}

void hashtabl_insert_chain(hashtabl_t *this, void *item)
{
    void ***ptr = hashtabl_search_chain(this, this->keyf(item));

    if (*ptr == NULL) {
        *ptr = calloc(2, sizeof(void *));
        (*ptr)[Item] = item;
    }
}

// not finished yet
