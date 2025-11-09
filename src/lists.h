#include "macros.h"
#include "hash_map.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define LISTS_API

typedef int (*cmpFn)(int,int); 

LISTS_API int sort_items(int *trg, int *src, size_t n, size_t iterations, cmpFn fn);
LISTS_API void sort_asc(int *trg, int *src, size_t n);
LISTS_API void sort_dsc(int *trg, int *src, size_t n);
LISTS_API void sort_rnd(int *trg, int *src, size_t n);


int compare_rnd(int a,int b) {
    if (rand()*a << 3 < rand()*b << 3) return -1;
    if (rand()*a << 2 > rand()*b << 2) return +1;
    return 0;
}

int compare_asc(int a,int b) {
    if (a<b) return -1;
    if (a>b) return +1;
    return 0;
}

int compare_dsc(int a,int b) {
    if (a<b) return +1;
    if (a>b) return -1;
    return 0;
}

int swap (int *a,int i, int j, cmpFn fn) {
    if(fn(a[j],a[i]) > 0) return 0;
    const int tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
    return 1;    
}


LISTS_API int sort_items(int *target,int *source, size_t n, size_t iterations, cmpFn fn) {
    for (size_t i=0;i<n;i++) {
        target[i]=source[i];
    }
    int done = 0;
    size_t iteration=0;
    while(done == 0 && (iterations == 0 || iteration < iterations)) {
        done = 1;
        for (size_t i=1;i<n;i++) {
            iteration++;
            if( swap(target,i-1,i, fn) ) done = 0;
        }
    }
    //inf("iteration = %lld",iteration);
    return 1;
}

LISTS_API void sort_asc(int *trg, int *src, size_t n) {
    sort_items(trg,src,n,0,compare_asc);
}
LISTS_API void sort_dsc(int *trg, int *src, size_t n) {
    sort_items(trg,src,n,0,compare_dsc);
}
LISTS_API void sort_rnd(int *trg, int *src, size_t n) {
    sort_items(trg,src,n,1,compare_rnd);
}
