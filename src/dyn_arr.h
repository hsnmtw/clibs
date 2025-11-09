#include <stdio.h>
#include <stdbool.h>
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

    DYN_ARR_API void dyn_arr_init(dyn_arr_t *arr);
    DYN_ARR_API void dyn_arr_append(dyn_arr_t *arr, char *value);
    DYN_ARR_API bool dyn_arr_contains(dyn_arr_t *dyn_arr, char *value);
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

    DYN_ARR_API void dyn_arr_init(dyn_arr_t *arr) {
        size_t size = sizeof(char*)*(DYN_ARR_INC_SIZE);
        char **items = (char**)malloc(size); 
        memset(items,0,size);
        arr->capacity = DYN_ARR_INC_SIZE;
    }

    DYN_ARR_API void dyn_arr_append(dyn_arr_t *arr, char *value) {
        if (arr == NULL || value == NULL) return;
        if (arr->items == NULL) dyn_arr_init(arr);
        if (arr->count+1 >= arr->capacity) {
            arr->items = (char**)realloc(arr->items,sizeof(char*)*(arr->capacity+DYN_ARR_INC_SIZE)); 
            memset(arr->items+arr->count,0,sizeof(char*)*(DYN_ARR_INC_SIZE));
            arr->capacity += DYN_ARR_INC_SIZE;
        }
        arr->items[arr->count++] = strdup(value);
    }

    DYN_ARR_API bool dyn_arr_contains(dyn_arr_t *arr, char *value) {
        if (arr == NULL || value == NULL || arr->items == NULL) return false;
        uint32_t hash = hash_map_hash(value);
        for(size_t i=0;i<arr->count;++i) {
            if (hash_map_hash(arr->items[i]) == hash) return true;
        }
        return false;
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

        assertf(dyn_arr_contains(&arr, "test"), "this should return true before clearing");
        
        dyn_arr_clear(&arr);
        assertf(arr.count == 0,"arr is cleared");
        assertf(arr.capacity == 0,"arr is cleared, capacity is zero");

        assertf(!dyn_arr_contains(&arr, "test"), "this should return false now");


        inf("[\033[1;44m%-30s\033[0m] tests", __func__);
    }

#   endif//__DYN_ARR_IMPL

#endif//__DYN_ARR_H