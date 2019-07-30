#include "bitio.h"

#define uintwid (sizeof(unsigned) * 8)

typedef struct {
    FILE *file;
    unsigned ibuf, ipos,
             obuf, opos;
} bfile_t;

void *bopen(FILE *file)
{
    bfile_t *this = malloc(sizeof(bfile_t));
    this->file = file;
    this->ibuf = 0;
    this->ipos = uintwid;
    this->obuf = 0;
    this->opos = 0;
}

int bget(void *_this, unsigned *pbits, unsigned nbits)
{
    bfile_t *this = _this;

    nbits = nbits <= uintwid ? nbits : uintwid;
    *pbits = (this->ipos != uintwid ? this->ibuf >> this->ipos : 0)
             & ((nbits != uintwid ? 1 << nbits : 0) - 1);
    this->ipos += nbits;
    if (this->ipos <= uintwid)
        return 0;

    this->ibuf = 0;
    this->ipos -= uintwid;
    unsigned rcnt = 8 * fread(&this->ibuf, 1, sizeof(unsigned), this->file);
    if (rcnt < this->ipos)
        return -1;

    *pbits |= (this->ibuf & ((this->ipos != uintwid ? 1 << this->ipos : 0) - 1))
              << (nbits - this->ipos);
    this->ibuf <<= uintwid - rcnt;
    this->ipos += uintwid - rcnt;
        return 0;
}

void bput(void *_this, unsigned bits, unsigned nbits)
{
    bfile_t *this = _this;

    nbits = nbits <= uintwid ? nbits : uintwid;
    if (nbits != uintwid) bits &= (1 << nbits) - 1;
    this->obuf |= bits << this->opos;
    this->opos += nbits;
    if (this->opos < uintwid)
        return;

    fwrite(&this->obuf, sizeof(unsigned), 1, this->file);
    this->opos -= uintwid;
    this->obuf = this->opos ? bits >> (nbits - this->opos) : 0;
}

void bflush(void *_this)
{
    bfile_t *this = _this;

    fwrite(&this->obuf, (this->ipos+7)/8, 1, this->file);
    this->obuf = this->opos = 0;
}
