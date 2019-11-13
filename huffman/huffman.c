#include <stdio.h>
#include <stdint.h>

#include "pqueue.h"
#include "bitio.h"
#include "huffman.h"

#define ROOT 0
#define LEAF 0x1ff

typedef struct {
    uint32_t occ;
    uint8_t  ch;
    uint8_t  bit;
    uint16_t par, br[2];
} node_t;

typedef struct {
    unsigned occ : 28;
    unsigned ch  :  8;
    unsigned bit :  1;
    unsigned par :  9;
    unsigned br0 :  9;
    unsigned br1 :  9;
} cnode_t;

#define nodehigher(n1, n2) (((node_t *)(n1))->occ < ((node_t *)(n2))->occ)

void writetree(node_t *tree, uint16_t nnod, FILE *file);

void huffman_encode(FILE *fdst, FILE *fsrc)
{
    node_t tree[0x200] = {};
    for (int ch; EOF != (ch = fgetc(fsrc)); )
        ++tree[ch].occ;

    uint16_t tmap[0x100], nnod = 0;
    for (int ch = 0; ch < 0x100; ++ch) {
        if (tree[ch].occ) {
            tree[nnod].occ = tree[ch].occ;
            tree[nnod].ch = ch;
            tree[nnod].br[0] = tree[nnod].br[1] = LEAF;
            tmap[ch] = nnod;
            ++nnod;
        }
    }

    PQ pq;
    PQinit(pq);

    for (int i = 0; i < nnod; ++i)
        PQpush(pq, tree+i, nodehigher);

    while (1) {
        node_t *nod0, *nod1;
        PQpop(pq, nod0, nodehigher);
        PQpop(pq, nod1, nodehigher);

        if (nod1 == NULL) {
            nod0->par = ROOT;
            break;
        }

        nod0->bit = 0;
        nod1->bit = 1;
        nod0->par = nod1->par = nnod;

        tree[nnod].br[0] = nod0 - tree;
        tree[nnod].br[1] = nod1 - tree;

        PQpush(pq, tree+nnod, nodehigher)

        ++nnod;
    }
    PQfree(pq);

    writetree(tree, nnod, fdst);

    bitio_t bout = BITIO_INIT;
    bout.file = fdst;

    rewind(fsrc);

    for (int ch; EOF != (ch = fgetc(fsrc)); )
    {
        int bits = 0, nbits = 0;

        for(int i = tmap[ch];
            tree[i].par != ROOT;
            bits = (bits << 1) | tree[i].bit,
            i = tree[i].par, ++nbits);

        bput(&bout, bits, nbits);
    }
    bflush(&bout);
}

uint16_t readtree(node_t *tree, FILE *file);

void huffman_decode(FILE *fdst, FILE *fsrc)
{
    node_t tree[0x200];
    uint16_t nnod;

    nnod = readtree(tree, fsrc);

    bitio_t bin = BITIO_INIT;
    bin.file = fsrc;

    while (1)
    {
        int i, bit;
        for(i = nnod-1;
            tree[i].br[0] != LEAF &&
            -1 != bget(&bin, &bit, 1);
            i = tree[i].br[bit]);

        if (tree[i].br[0] != LEAF) break;

        fputc(tree[i].ch, fdst);
    }
}

void writetree(node_t *tree, uint16_t nnod, FILE *file)
{
    fwrite(&nnod, sizeof(uint16_t), 1, file);

    for (int i = 0; i < nnod; ++i) {
        cnode_t cnode;
        cnode.occ = tree[i].occ;
        cnode.ch  = tree[i].ch;
        cnode.bit = tree[i].bit;
        cnode.par = tree[i].par;
        cnode.br0 = tree[i].br[0];
        cnode.br1 = tree[i].br[1];
        fwrite(&cnode, sizeof(cnode_t), 1, file);
    }
}

uint16_t readtree(node_t *tree, FILE *file)
{
    uint16_t nnod;
    fread(&nnod, sizeof(uint16_t), 1, file);

    for (int i = 0; i < nnod; ++i) {
        cnode_t cnode;
        fread(&cnode, sizeof(cnode_t), 1, file);
        tree[i].occ   = cnode.occ;
        tree[i].ch    = cnode.ch;
        tree[i].bit   = cnode.bit;
        tree[i].par   = cnode.par;
        tree[i].br[0] = cnode.br0;
        tree[i].br[1] = cnode.br1;
    }

    return nnod;
}
