#include <stdio.h>
#include <string.h>
#include "macros.h"
#include <stdlib.h>

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


    DYN_ARR_API void test_dyn_arr_hash(void) {

    }

#   endif//__DYN_ARR_IMPL

#endif//__DYN_ARR_H