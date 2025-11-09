#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "macros.h"
#include "macros.h"

#pragma once

#ifndef __SET_H
#define __SET_H

#ifndef SET_API
#define SET_API
#endif//SET_API

#define SET_INC_SIZE 50
//
// --------------------------------------------------------------------
//    INTERFACE
// --------------------------------------------------------------------
//

    typedef struct {
        char **items;
        uint32_t *hashes;
        size_t count;
        size_t capacity;
    } set_t;

    SET_API void set_append(set_t *set, char *value);
    SET_API bool set_contains(set_t *set, char *value);
    SET_API char *set_get(set_t *set, size_t index);
    SET_API void set_remove_at(set_t *set, size_t index);
    SET_API void set_clear(set_t *set);


//
// --------------------------------------------------------------------
//    IMPLEMENTATION
// --------------------------------------------------------------------
//

#   ifndef __SET_IMPL
#   define __SET_IMPL

    SET_API void set_init(set_t *arr) {
        size_t size = sizeof(char*)*(DYN_ARR_INC_SIZE);
        char **items = (char**)malloc(size); 
        memset(items,0,size);
        arr->capacity = DYN_ARR_INC_SIZE;
        arr->hashes = (uint32_t*)malloc(sizeof(uint32_t)*(arr->capacity));

    }

    SET_API bool set_contains(set_t *set, char *value) {
        if (set == NULL || value == NULL || set->items == NULL || set->hashes == NULL) return false;
        uint32_t hash = hash_map_hash(value);
        for(size_t i=0;i<set->count;++i) {
            if (set->hashes[i] == hash) return true;
        }
        return false;
    }

    SET_API void set_append(set_t *set, char *value) {
        if (set == NULL || value == NULL) return;
        if (set->items == NULL || set->hashes==NULL) set_init(set);
        if (set_contains(set,value)) return;
        if (set->count+1 >= set->capacity) {
            set->items = (char**)realloc(set->items,sizeof(char*)*(set->capacity+SET_INC_SIZE)); 
            set->hashes = (uint32_t*)realloc(set->hashes,sizeof(uint32_t)*(set->capacity+SET_INC_SIZE));
            memset(set->items+set->count,0,sizeof(char*)*(SET_INC_SIZE));
            memset(set->hashes+set->count,0,sizeof(uint32_t)*(SET_INC_SIZE));
            set->capacity += SET_INC_SIZE;
        }
        set->hashes[set->count] = hash_map_hash(value);
        set->items[set->count] = strdup(value);
        set->count += 1;
    }

    SET_API char *set_get(set_t *set, size_t index) {
        if (set == NULL || set->items == NULL) return NULL;
        if (index >= set->capacity) return NULL;
        return set->items[index];
    }

    SET_API void set_remove_at(set_t *set, size_t index) {
        if (set == NULL || set->items == NULL) return;
        if (index >= set->capacity) return;
        if(set->items[index] != NULL) free(set->items[index]);
    }

    SET_API void set_clear(set_t *set) {
        if (set == NULL || set->items == NULL) return;
        for(size_t i=0;i<set->capacity;++i) {
            if(set->items[i] != NULL) free(set->items[i]);
        }
        memset(set->items,0,sizeof(char*)*set->capacity);
        free(set->items);
        set->items = NULL;
        set->count = 0;
        set->capacity = 0;
    }

    SET_API void test_set(void) {
        set_t set = {0};
        assertf(set.count == 0,"set is null initially");
        assertf(set.capacity == 0,"set is null initially");
        set_append(&set,"my");
        set_append(&set,"first");
        set_append(&set,"dyn");
        assertf(set.count == 3,"set is 3 items now");
        set_append(&set,"set");
        set_append(&set,"test");
        assertf(set.count == 5,"set is 5 items now");

        assertf(strcmp(set_get(&set,2),"dyn")==0,"test get by index");
        assertf(set_get(&set,10)==NULL,"test get by index NULL item");

        
        assertf(set.capacity==50,"should have capacity = 50");
        for(size_t i=0;i<100;++i) set_append(&set,"test");
        assertf(set.count==5,"should have 5 items now, but got %lld", set.count);
        assertf(set.capacity==50,"should have capacity of 150, but got %lld", set.capacity);
        
        assertf(set_contains(&set, "test"), "this should return true before clearing");
        set_clear(&set);
        assertf(set.count == 0,"set is cleared");
        assertf(set.capacity == 0,"set is cleared, capacity is zero");

        assertf(!set_contains(&set, "test"), "this should return false now");

        inf("[\033[1;44m%-30s\033[0m] tests", __func__);
    }

#   endif//__SET_IMPL

#endif//__SET_H