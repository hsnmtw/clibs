#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "macros.h"

#pragma once

#ifndef __DYN_ARR_H
#define __DYN_ARR_H

#ifndef DYN_ARR_API
#define DYN_ARR_API
#endif//DYN_ARR_API

#define DYN_ARR_INC_SIZE 50
//
// --------------------------------------------------------------------
//    INTERFACE
// --------------------------------------------------------------------
//

    typedef struct {
        char **items;
        size_t count;
        size_t capacity;
    } dyn_arr_t;

    DYN_ARR_API void dyn_arr_append(dyn_arr_t *arr, char *value);
    DYN_ARR_API char *dyn_arr_get(dyn_arr_t *arr, size_t index);
    DYN_ARR_API void dyn_arr_remove_at(dyn_arr_t *arr, size_t index);
    DYN_ARR_API void dyn_arr_clear(dyn_arr_t *arr);


//
// --------------------------------------------------------------------
//    IMPLEMENTATION
// --------------------------------------------------------------------
//

#   ifndef __DYN_ARR_IMPL
#   define __DYN_ARR_IMPL

    DYN_ARR_API void dyn_arr_append(dyn_arr_t *arr, char *value) {
        if (arr->count >= arr->capacity) {
            size_t size = sizeof(char*)*(arr->count+DYN_ARR_INC_SIZE);
            char **items = (char**)malloc(size); 
            memset(items,0,size);
            if (arr->items != NULL) {
                for(size_t i=0;i<arr->capacity;++i) {
                    if (arr->items[i] == NULL) continue;
                    items[i] = strdup(arr->items[i]);
                    free(arr->items[i]);
                }
                free(arr->items);
            }
            arr->items = items;
            arr->capacity += DYN_ARR_INC_SIZE;
        }
        arr->items[arr->count++] = strdup(value);
    }

    DYN_ARR_API char *dyn_arr_get(dyn_arr_t *arr, size_t index) {
        if (arr == NULL || arr->items == NULL) return NULL;
        if (index >= arr->capacity) return NULL;
        return arr->items[index];
    }

    DYN_ARR_API void dyn_arr_remove_at(dyn_arr_t *arr, size_t index) {
        if (arr == NULL || arr->items == NULL) return;
        if (index >= arr->capacity) return;
        if(arr->items[index] != NULL) free(arr->items[index]);
    }

    DYN_ARR_API void dyn_arr_clear(dyn_arr_t *arr) {
        if (arr == NULL || arr->items == NULL) return;
        for(size_t i=0;i<arr->capacity;++i) {
            if(arr->items[i] != NULL) free(arr->items[i]);
        }
        memset(arr->items,0,sizeof(char*)*arr->capacity);
        free(arr->items);
        arr->items = NULL;
        arr->count = 0;
        arr->capacity = 0;
    }

    DYN_ARR_API void test_dyn_arr(void) {
        dyn_arr_t arr = {0};
        assertf(arr.count == 0,"arr is null initially");
        assertf(arr.capacity == 0,"arr is null initially");
        dyn_arr_append(&arr,"my");
        dyn_arr_append(&arr,"first");
        dyn_arr_append(&arr,"dyn");
        assertf(arr.count == 3,"arr is 3 items now");
        dyn_arr_append(&arr,"array");
        dyn_arr_append(&arr,"test");
        assertf(arr.count == 5,"arr is 5 items now");

        assertf(strcmp(dyn_arr_get(&arr,2),"dyn")==0,"test get by index");
        assertf(dyn_arr_get(&arr,10)==NULL,"test get by index NULL item");

        
        assertf(arr.capacity==50,"should have capacity = 50");
        for(size_t i=0;i<100;++i) dyn_arr_append(&arr,"test");
        assertf(arr.count==105,"should have 105 items now");
        assertf(arr.capacity==150,"should have capacity of 150, but got %lld", arr.capacity);
        
        dyn_arr_clear(&arr);
        assertf(arr.count == 0,"arr is cleared");
        assertf(arr.capacity == 0,"arr is cleared, capacity is zero");

        inf("[\033[1;44m%-30s\033[0m] tests, all = %d --- success = %d", __func__, 8, 8);
    }

#   endif//__DYN_ARR_IMPL

#endif//__DYN_ARR_H