#include <stdlib.h>
#include <string.h>
 
#define swap(a,b,s) {   \
    char _t[s];         \
    memcpy(_t,  a,  s); \
    memcpy( a,  b,  s); \
    memcpy( b, _t,  s); \
}
 
void *iord(int no, void *arr, size_t num, size_t sz, int (*cmp)(void*,void*))
{
    int part_(void *arr, size_t num, size_t sz, int (*cmp)(void*,void*))
    {
        char (*arr_)[sz] = arr;
        int r = rand() % num, k = num - 1;
        swap(arr_[r], arr_[k], sz);
 
        int i, j;
        for (i = 0, j = 0; j < k; ++j) {
            if (cmp(arr_[j], arr_[k]) < 0) {
                swap(arr_[i], arr_[j], sz);
                ++i;
            }
        }
        swap(arr_[i], arr_[k], sz);
 
        return i;
    }
 
    void pair_(void *arr, size_t num, size_t sz, int (*cmp)(void*,void*))
    {
        char (*arr_)[sz] = arr;
 
        for (int i = 0; i+1 < num; i += 2) {
            if (cmp(arr_[i], arr_[i+1]) > 0)
                swap(arr_[i], arr_[i+1], sz);
        }
    }
 
    char (*arr_)[sz] = arr;
 
    if (no+1 < num/2) {
        pair_(arr_, num, sz, cmp);
        char (*_arr)[sz] = iord(no, arr_, num/2, 2*sz, cmp);
        if (num % 2) {
            ++_arr;
            swap(_arr, arr_[num-1], sz);
        }
        return iord(no, arr_, _arr - arr_, sz, cmp);
    } else {
        int mid = part_(arr_, num, sz, cmp);
        return no == mid++ ? arr_[no]
             : no <  mid   ? iord(no, arr_, mid, sz, cmp)
                           : iord(no-mid, arr_[mid], num-mid, sz, cmp);
    }
}
