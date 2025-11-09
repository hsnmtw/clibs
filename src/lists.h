#include "macros.h"
#include <stdlib.h>
#include <string.h>

#define LISTS_API

typedef int (*cmpFn)(int,int); 

LISTS_API int sort_items(int *target,int *items, size_t n, cmpFn fn);

int swap (int *a,int i, int j, cmpFn fn) {
    if(fn(a[j],a[i]) > 0) return 0;
    const int tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
    return 1;    
}

LISTS_API int sort_items(int *target,int *source, size_t n, cmpFn fn) {
    for (size_t i=0;i<n;i++) {
        target[i]=source[i];
    }
    int done = 0;
    int iteration=0;
    while(done == 0) {
        done = 1;
        for (size_t i=1;i<n;i++) {
            iteration++;
            if( swap(target,i-1,i, fn) ) done = 0;
        }
    }
    inf("iteration = %d",iteration);
    return 1;
}