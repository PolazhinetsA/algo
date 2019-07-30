/* a - source array;
 * b - destination array;
 * c - user supplied storage for counting, if not NULL;
 * n - array size;
 * r - range of values key values;
 * k - key extractor.
 */
#define rsort_macro(a,b,c,n,r,k) {      \
    int _t[r], *_c = (c) ? (c) : _t;    \
    memset(_c, 0, sizeof(_t));          \
                                        \
    for (int _i = 0; _i < (n); ++_i) {  \
        ++_c[k((a)[_i])];               \
    for (int _i = 1; _i < (r); ++_i) {  \
        _c[_i] += _c[_i-1];             \
    for (int _i = n-1; _i >= 0; --_i) { \
        (b)[--_c[k((a)[_i])]] = (a)[_i];\
}
#define bits(x,b_,_b) (*(unsigned long long *)&(x) >> (b_) & ((1 << (_b)) - 1))
#define swap(a,b) { typeof(a) _t = a; a = b; b = _t; }

/* count sort by least significant byte 0..7,
 * then by 8..15 and so on...
 */
void rsort_int(int *arr, int num)
{
    int *arr_ = malloc(num * sizeof(int));

    for (int shr = 0; shr < sizeof(int) * 8; shr += 8) {
        #define key(x) bits(x,shr,8)
        rsort_macro(arr, arr_, NULL, num, 0x100, key);
        #undef key
        swap(arr, arr_);
    }

    free(arr_);
}

/* radix sorting of double precision floating point numbers:
 * 1. by mantissa, just like integers;
 * 2. by exponent, just like integers;
 * (1.111... * 2^n < 1.000... * 2^(n+1))
 * 3. when sorted by absolute value, collect in reverse order,
 *    stack negative ones on the left side
 *    (descending by absolute value == ascending by signed value),
 *    and positive ones on the right side
 *    (descending-descending == ascending).
 */
void rsort_dbl(double *arr, int num)
{
    double *arr_ = malloc(num * sizeof(double));

    for (int shr = 0; shr < 52; shr += 13) {
        #define key(x) bits(x,shr,13)
        rsort_macro(arr, arr_, NULL, num, 0x2000, key);
        #undef key
        swap(arr, arr_);
    }
    #define key(x) bits(x,52,11)
    rsort_macro(arr, arr_, NULL, num, 0x800, key);
    #undef key
    swap(arr, arr_);

    for (int i = num-1, i_ = 0, _i = num-1; i >= 0; --i) {
        if (arr[i] < 0) arr_[i_++] = arr[i];
                   else arr_[_i--] = arr[i];
    }
    swap(arr, arr_);
  
    free(arr_);
}

/* sorting of strings:
 * 1. find maxlen
 * 2. sort by len
 * 3. sort trailing subarray of ones not shorter than maxlen
 *    by maxlen-1st letter
 * 4. sort trailing subarray of ones not shorter than maxlen-1
 *    by maxlen-2nd letter
 * 5. ...
 * 6. by 0th letter
 */
void rsort_str(unsigned char **arr, int num)
{
    // avoid computing same strlen twice
    struct str {
        unsigned char *ptr;
        size_t         len;
    } *arr_  = malloc(num * sizeof(struct str)),
      *arr__ = malloc(num * sizeof(struct str));

    int maxl = 0;
    for (int i = 0; i < num; ++i) {
        arr_[i].ptr = arr[i];
        arr_[i].len = strlen(arr[i]);
        if (maxl < arr_[i].len)
            maxl = arr_[i].len;
    }

    /* for l=0..maxl: lidx[l] will become the index of the first string not
     * shorter than l, when used for counting. Passing decremented pointer
     * would do the trick, if there was guarantee, that no zero-length strings
     * occur.
     */
    int lidx[maxl+1];
    #define key(x) ((x).len)
    rsort_macro(arr_, arr__, lidx, num, maxl+1, key);
    #undef key
    swap(arr_, arr__);

    for (int l = maxl, i = l-1; l > 0; --l, --i) {
        memcpy(arr__, arr_, lidx[l] * sizeof(struct str));
        #define key(x) ((x).ptr[i])
        rsort_macro(&arr_[lidx[l]], &arr__[lidx[l]], NULL,
                    num - lidx[l], 0x100, key);
        #undef key
        swap(arr_, arr__);
    }

    for (int i = 0; i < num; ++i) {
        arr[i] = arr_[i].ptr;
    }
    free(arr__);
    free(arr_);
}
