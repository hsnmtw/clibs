#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "macros.h"

#pragma once

#define MAP_INCREMENT_SIZE 123


#ifndef __HASH_MAP_H
#define __HASH_MAP_H

#ifndef HASH_MAP_API
#define HASH_MAP_API
#endif//HASH_MAP_API

static size_t miss = 0;
static size_t mizz = 0;

//
// --------------------------------------------------------------------
//    INTERFACE
// --------------------------------------------------------------------
//
    typedef struct entry_t {
        char *key;
        int   val;
    } entry_t;

    typedef struct hash_map_t {
        entry_t *items;
        size_t count;
        size_t capacity;
    } hash_map_t;


    // perform a hash of a key/text gets a number
    HASH_MAP_API uint32_t  hash_map_hash(const char *text);

    // if the key == NULL, or empty string, return INT_MIN
    // attempts to locate the index of the given key if available in the map, 
    // if the map is full and key is not available (count==capacity), return -ve index (negative)
    // if key is unavailable, returns newly empty item index
    HASH_MAP_API int  hash_map_index(hash_map_t *map, const char *key);

    // if the entries set is NULL or count == 0 or capacity == 0, resize, and make capacity doubled
    // after every add, the count is incremented
    // side effects: 
    //      - map items memory allocation is modified
    //      - capacity is incremented
    //      - count is incremented
    HASH_MAP_API void hash_map_add(hash_map_t *map, const char *key, const int val);

    // removes an item from map if available only perform side effects
    // side effects: 
    //      - count is decremented
    HASH_MAP_API void hash_map_remove(hash_map_t *map, const char *key);
    // removed all items and reset count to zero
    // side effects: 
    //      - map items memory allocation is freed
    //      - capacity is set to zero
    //      - count is set to zero
    HASH_MAP_API void hash_map_clear(hash_map_t *map);
    
    HASH_MAP_API int hash_map_get(hash_map_t *map, const char *key);

    HASH_MAP_API void test_hash_map_hash(void);

//
// --------------------------------------------------------------------
//    IMPLEMENTATION
// --------------------------------------------------------------------
//

#   ifndef __HASH_MAP_IMPL
#   define __HASH_MAP_IMPL

    #define HASH_FACTOR 391993711
    #define PRIMES_LEN 123
    const uint32_t primes[] = {
        193939,3,263,76801,293,359,18253,1201,1931,37199,193,391939,393919,13873,101,353,9311,
        71,28813,993319,389,939391,233,19,307,971,83,4801,65713,127,191,269,991,7793,211,227,
        113,41,257,107,179,2,251,37,12289,23,157,71993,137,149,43201,9377,10093,20173,401,719,
        17,733,67,11,919,3119,769,19391,99371,115249,112909,167,53,47629,93911,84673,1193,999331,
        311,3779,3469,197,69313,3889,11939,37633,21169,337,93719,281,139,106033,131,91193,379,89,
        73009,19937,109,39119,317,60493,939193,199,7937,1453,29,31,933199,59,181,319993,22189,
        331999,347,47,5,239,13,2029,433,108301,919393,7,63949,199933,409,
    };

    HASH_MAP_API uint32_t hash_map_hash(const char *text) {
        if (text == NULL) return INT_MIN;
        size_t n = strlen(text);
        long long sum = 0;
        for(size_t i=0;i<n;++i) {
            sum += sqrt(pow(primes[((uint32_t)text[i])%PRIMES_LEN],n-i));
        }
        return (uint32_t) (sum < 0 ? -sum : sum) % HASH_FACTOR;
    }

    //[TRC] : misses on hash lookup = 205056

    HASH_MAP_API void hash_map_add(hash_map_t *map, const char *key, const int val) {
        assertf(map != NULL, " hash map is null ");
        assertf(key != NULL || strcmp(key, "")==0, " the key cannot be NULL or empty ");
        //todo("%s",__func__);
        if (map->items == NULL || map->items == 0 || map->capacity < 1 || map->count + 1 >= map->capacity) {
            
            if(map->items == NULL              ) dbg(" map->items == NULL ");
            if(map->items == 0                 ) dbg(" map->items == 0 ");
            if(map->capacity < 1               ) dbg(" map->capacity < 1 ");
            if(map->count + 1 >= map->capacity ) dbg(" map->count + 1 >= map->capacity ");

            dbg("expand ... %zu -- %zu || %d", map->count, map->capacity, map->count + 1 >= map->capacity);
            
            size_t n = map->capacity + MAP_INCREMENT_SIZE;
            entry_t *items = (entry_t*)malloc(sizeof(entry_t)*n);
            hash_map_t nmap = { .items = items, .count = map->count, .capacity = n };
            memset(items,0,sizeof(entry_t)*n);
            size_t c=0;
            if (map->items != NULL) {
                for(size_t i=0;i<map->capacity;++i) {
                    if (map->items[i].key==NULL) continue;
                    items[hash_map_index(&nmap,map->items[i].key)] = map->items[i];//(entry_t) {strdup(map->items[i].key), map->items[i].val};
                    c += 1;
                }
                free(map->items);
            }
            map->capacity = n;
            assertf(map->count == c, "count is not same %zu // %zu", map->count, c);
            map->items = items;
        }

        int index = hash_map_index(map, key);
        assertf (index>-1 && index < (int)map->capacity,"index=%d , count = %zu, capacity = %zu", index, map->count, map->capacity)
        assertf (map->items[index].key == NULL || strcmp(map->items[index].key, key) == 0, "expect item at index %d to be NULL or of key = `%s`", index, key );
        assertf (map->capacity > 0, "map capacity must be more than zero");
        assertf (map->count < map->capacity, "map count must be non-negative");
        // assertf (index >=0 && index < (int)map->capacity, " index must be between [%d,%d] , the given value was %d", 0, (int)map->capacity, index);
        map->items[index] = (entry_t) {strdup(key),val};
        dbg ("%-13s : %d", key, index);
        map->count += 1;
        dbg("count = %zu,, cap=%zu", map->count, map->capacity);
    }

    HASH_MAP_API int  hash_map_is_key_used(hash_map_t *map, const char *key, int index) {
        assertf(map != NULL, " hash map is null ");
        assertf(map->capacity > 0, " the hash map is not initialized yet ");
        assertf(index < (int)map->capacity && index >= 0," expect the index to be in range [0, %zu], the provided index is %d", map->capacity - 1, index);
        
        return map->items[index].key != NULL && strcmp(key, map->items[index].key)!=0;
    }
    
    HASH_MAP_API int  hash_map_index(hash_map_t *map, const char *key) {
        assertf (map != NULL                         , " hash map is null ");
        assertf (map->items != NULL                  , " hash map items is null ");        
        assertf (map->capacity > 0                   , " capacity [%zu] is less than 1, count=%zu", map->capacity, map->count);
        assertf (key != NULL && strcmp(key, "") != 0 , " key must be NON-NULL and NOT EMPTY !");        
        assertf (map->count <= map->capacity         , " map->count IS MORE THAN map->capacity ");
        
        int n=map->capacity;
        int index = hash_map_hash(key) % n;
        int u=index,d=index+1;
        
        if (u>-1 && map->items[u].key != NULL && strcmp(map->items[u].key, key) == 0) return u;
        mizz++;
        for ( int i=0; i < n && (u>-1 || d<n); ++i ) {
            if (u>-1 && map->items[u].key != NULL && strcmp(map->items[u].key, key) == 0) return u;
            if (d<n  && map->items[d].key != NULL && strcmp(map->items[d].key, key) == 0) return d;
            
            u--;
            d++;
            miss++;
        }
        
        u=index,d=index+1;
        for ( int i=0; i < n && (u>-1 || d<n); ++i ) {
            if (u>-1 && map->items[u].key == NULL) return u;
            if (d<n  && map->items[d].key == NULL) return d;
            u--;
            d++;
            miss++;
        }

        return -1;
    }

    HASH_MAP_API void hash_map_remove(hash_map_t *map, const char *key) {
        assertf(map != NULL, " hash map is null ");
        assertf(map->capacity > 0, " the hash map is not initialized yet ");
        assertf(key != NULL || strcmp(key, "")==0, " the key cannot be NULL or empty ");
        
        assertf(map->count > 1, " no items added to the map ");
        assertf(map->capacity > 1, " the capacity of the map must be positive and greater than zero ");

        int index = hash_map_index(map,key);
        if (index >=0 && index < (int)map->capacity) {
            map->items[index].key = NULL;
            map->items[index].val = INT_MIN;
            map->count--;
        }
    }

    HASH_MAP_API void hash_map_clear(hash_map_t *map) {
        if (map->capacity > 0 && map->count > 0) free(map->items);
        map->count = 0;
        map->capacity = 0;
    }

    HASH_MAP_API int hash_map_get(hash_map_t *map, const char *key) {
        int index = hash_map_index(map,key);
        if (index < 0 || index >= (int)map->capacity) return INT_MIN;
        return map->items[index].val;
    }


    HASH_MAP_API void test_hash_map_hash(void) {

        FILE *file = fopen("./AbigailsTale.txt", "r");
        if (file == NULL) {
            err("failed to open file ... 1");
            fclose(file);
            return;
        }
        char buffer[4096] = {0};
        size_t r;
        if ((r = fread(buffer,sizeof(char),4096,file)) < 1) {
            err("failed to open file ... 2");
            fclose(file);
            return;
        }

        size_t spaces = 0;
        bool in_space = false;
        for(size_t i=0;i<4096 && buffer[i] != '\0';++i) {
            if (isspace(buffer[i])) {
                if (!in_space) spaces++;
                in_space = true;
            } else {
                in_space = false;
            }
        }

        inf("number of words = %zu", spaces+1);

        assertf(spaces == 683, "number of spaces must be 683, but got = %zu", spaces);

        char *words[spaces+1];
        size_t j = 0, w = 0;
        for(size_t i=0;i<4096 && buffer[i] != '\0';++i) {
            if (isspace(buffer[i])) continue;
            char word[40] = {0};
            while(!isspace(buffer[i]) && j<40) {
                word[j++] = buffer[i++];
            }
            // inf("w='%s'",word);
            words[w++] = strdup(word);
            j=0;
        }

        // if(spaces>0) return 1;


        fclose(file);

        static hash_map_t map = {NULL,0,0};
        size_t n = ARRAY_LEN(words);
        for(size_t i=0;i < n;++i) {
            char *key = words[i];
            if (map.count < 1 || hash_map_get(&map,key) < 1) {
                hash_map_add(&map,key,1);
            } else {
                int index = hash_map_index(&map, key);
                //dbg("key=%s -- index=%d",key,index);
                map.items[index].val += 1;
            }
        }

        inf("number of items: %zu", map.count);
        FILE *fout = fopen("./tale.txt","w");
        for(size_t i=0;i < map.capacity;++i) {
            if (map.items[i].key == NULL) continue;
            char line[100] = {0};
            sprintf(line, "%-20s | %d\n", map.items[i].key, map.items[i].val);
            //sprintf(line,"key = '%s', freq=%d, index='%zu'\n", map.items[i].key, map.items[i].freq, i);
            fwrite(line,sizeof(char),strlen(line),fout);
        }
        fclose(fout);
        assertf(map.count==308,"we should have 308 items, but got %zu", map.count);


        const entry_t data[] = {
            {.key = "1After"     , .val = 1 },
            {.key = "Abi"        , .val = 14},
            {.key = "Abi,"       , .val = 2 },
            {.key = "Abigail"    , .val = 3 },
            {.key = "Abigail's"  , .val = 5 },
            {.key = "Abigail."   , .val = 1 },
            {.key = "And"        , .val = 1 },
            {.key = "As"         , .val = 1 },
            {.key = "Aww"        , .val = 1 },
            {.key = "Bob"        , .val = 3 },
            {.key = "Bob's"      , .val = 1 },
            {.key = "Bob,"       , .val = 1 },
            {.key = "Everyday"   , .val = 1 },
            {.key = "He"         , .val = 1 },
            {.key = "I"          , .val = 1 },
            {.key = "Not"        , .val = 1 },
            {.key = "Of"         , .val = 1 },
            {.key = "Once"       , .val = 1 },
            {.key = "One"        , .val = 1 },
            {.key = "She"        , .val = 7 },
            {.key = "Sinbad!\""  , .val = 1 },
            {.key = "Sinbad"     , .val = 5 },
            {.key = "Sinbad,"    , .val = 1 },
            {.key = "Sinbad."    , .val = 3 },
            {.key = "So"         , .val = 1 },
            {.key = "Tale"       , .val = 1 },
            {.key = "Tears"      , .val = 1 },
            {.key = "The"        , .val = 1 },
            {.key = "Then"       , .val = 2 },
            {.key = "They"       , .val = 1 },
            {.key = "Tom"        , .val = 7 },
            {.key = "Tom's"      , .val = 2 },
            {.key = "Tom,"       , .val = 1 },
            {.key = "Tom."       , .val = 2 },
            {.key = "Tom\"."     , .val = 1 },
            {.key = "\"I"        , .val = 2 },
            {.key = "\"Please"   , .val = 1 },
            {.key = "\"Please\"" , .val = 1 },
            {.key = "\"WHAT!\""  , .val = 1 },
            {.key = "\"well\""   , .val = 1 },
            {.key = "\"you"      , .val = 2 },
            {.key = "a"          , .val = 9 },
            {.key = "about"      , .val = 2 },
            {.key = "across"     , .val = 6 },
            {.key = "acted."     , .val = 1 },
            {.key = "advice"     , .val = 1 },
            {.key = "advice,"    , .val = 2 },
            {.key = "advice."    , .val = 1 },
            {.key = "after"      , .val = 1 },
            {.key = "again."     , .val = 1 },
            {.key = "also"       , .val = 1 },
            {.key = "and"        , .val = 33},
            {.key = "angry"      , .val = 1 },
            {.key = "around"     , .val = 3 },
            {.key = "arrived"    , .val = 1 },
            {.key = "as"         , .val = 3 },
            {.key = "ask"        , .val = 2 },
            {.key = "at"         , .val = 2 },
            {.key = "away"       , .val = 1 },
            {.key = "badly,"     , .val = 1 },
            {.key = "badly."     , .val = 1 },
            {.key = "banged"     , .val = 1 },
            {.key = "banks"      , .val = 1 },
            {.key = "be"         , .val = 2 },
            {.key = "beat"       , .val = 1 },
            {.key = "before"     , .val = 1 },
            {.key = "begged"     , .val = 1 },
            {.key = "best"       , .val = 1 },
            {.key = "between"    , .val = 1 },
            {.key = "boat"       , .val = 3 },
            {.key = "both"       , .val = 1 },
            {.key = "boyfriend"  , .val = 1 },
            {.key = "boyfriend," , .val = 2 },
            {.key = "boys"       , .val = 1 },
            {.key = "bridge"     , .val = 4 },
            {.key = "bridge,"    , .val = 1 },
            {.key = "built,"     , .val = 1 },
            {.key = "but"        , .val = 6 },
            {.key = "called"     , .val = 2 },
            {.key = "can"        , .val = 1 },
            {.key = "cheated"    , .val = 1 },
            {.key = "cheating"   , .val = 1 },
            {.key = "confused"   , .val = 1 },
            {.key = "continued"  , .val = 1 },
            {.key = "could"      , .val = 1 },
            {.key = "couldn't"   , .val = 2 },
            {.key = "course"     , .val = 1 },
            {.key = "crazy."     , .val = 1 },
            {.key = "cried"      , .val = 4 },
            {.key = "cried."     , .val = 1 },
            {.key = "cross"      , .val = 2 },
            {.key = "crossing"   , .val = 1 },
            {.key = "crying."    , .val = 1 },
            {.key = "cup"        , .val = 1 },
            {.key = "days"       , .val = 1 },
            {.key = "decide"     , .val = 1 },
            {.key = "decided"    , .val = 1 },
            {.key = "decision"   , .val = 1 },
            {.key = "did"        , .val = 1 },
            {.key = "didn't!"    , .val = 1 },
            {.key = "didn't"     , .val = 3 },
            {.key = "disgusted"  , .val = 2 },
            {.key = "do,"        , .val = 1 },
            {.key = "do."        , .val = 1 },
            {.key = "door"       , .val = 2 },
            {.key = "down"       , .val = 1 },
            {.key = "each"       , .val = 2 },
            {.key = "ends."      , .val = 1 },
            {.key = "ever"       , .val = 1 },
            {.key = "ever..."    , .val = 1 },
            {.key = "everyone"   , .val = 1 },
            {.key = "everything" , .val = 2 },
            {.key = "explain"    , .val = 1 },
            {.key = "explained"  , .val = 4 },
            {.key = "face,"      , .val = 1 },
            {.key = "fact"       , .val = 1 },
            {.key = "felt"       , .val = 2 },
            {.key = "few"        , .val = 1 },
            {.key = "finished"   , .val = 1 },
            {.key = "flooded"    , .val = 1 },
            {.key = "for"        , .val = 6 },
            {.key = "friend,"    , .val = 1 },
            {.key = "frightful"  , .val = 1 },
            {.key = "gave"       , .val = 1 },
            {.key = "girl"       , .val = 1 },
            {.key = "go"         , .val = 1 },
            {.key = "great"      , .val = 1 },
            {.key = "had"        , .val = 4 },
            {.key = "hands"      , .val = 1 },
            {.key = "hands."     , .val = 1 },
            {.key = "happened"   , .val = 2 },
            {.key = "happy"      , .val = 1 },
            {.key = "he"         , .val = 8 },
            {.key = "help"       , .val = 1 },
            {.key = "help."      , .val = 1 },
            {.key = "her"        , .val = 18},
            {.key = "her,"       , .val = 2 },
            {.key = "herself."   , .val = 1 },
            {.key = "him"        , .val = 2 },
            {.key = "him,"       , .val = 2 },
            {.key = "him."       , .val = 2 },
            {.key = "his"        , .val = 4 },
            {.key = "hold"       , .val = 1 },
            {.key = "holding"    , .val = 1 },
            {.key = "house"      , .val = 4 },
            {.key = "how"        , .val = 2 },
            {.key = "if"         , .val = 1 },
            {.key = "in"         , .val = 5 },
            {.key = "into"       , .val = 2 },
            {.key = "is"         , .val = 1 },
            {.key = "it"         , .val = 1 },
            {.key = "it,"        , .val = 1 },
            {.key = "it."        , .val = 1 },
            {.key = "jumped"     , .val = 1 },
            {.key = "kept"       , .val = 1 },
            {.key = "knew"       , .val = 1 },
            {.key = "know"       , .val = 1 },
            {.key = "known"      , .val = 1 },
            {.key = "least"      , .val = 1 },
            {.key = "left"       , .val = 1 },
            {.key = "let"        , .val = 1 },
            {.key = "listened"   , .val = 1 },
            {.key = "little"     , .val = 2 },
            {.key = "lived"      , .val = 1 },
            {.key = "lived."     , .val = 1 },
            {.key = "looked"     , .val = 1 },
            {.key = "looking"    , .val = 1 },
            {.key = "love"       , .val = 1 },
            {.key = "love."      , .val = 1 },
            {.key = "loved"      , .val = 1 },
            {.key = "lovely!"    , .val = 1 },
            {.key = "lovely."    , .val = 1 },
            {.key = "make"       , .val = 1 },
            {.key = "man"        , .val = 2 },
            {.key = "many"       , .val = 1 },
            {.key = "me"         , .val = 1 },
            {.key = "me..."      , .val = 1 },
            {.key = "me.\""      , .val = 1 },
            {.key = "months"     , .val = 1 },
            {.key = "more"       , .val = 2 },
            {.key = "more,"      , .val = 1 },
            {.key = "morning"    , .val = 1 },
            {.key = "mother"     , .val = 5 },
            {.key = "mother,"    , .val = 1 },
            {.key = "mother."    , .val = 1 },
            {.key = "much"       , .val = 2 },
            {.key = "my"         , .val = 1 },
            {.key = "need"       , .val = 1 },
            {.key = "needed"     , .val = 1 },
            {.key = "new"        , .val = 1 },
            {.key = "next"       , .val = 1 },
            {.key = "nice"       , .val = 1 },
            {.key = "night"      , .val = 1 },
            {.key = "no"         , .val = 2 },
            {.key = "now"        , .val = 2 },
            {.key = "of"         , .val = 3 },
            {.key = "off"        , .val = 1 },
            {.key = "offered"    , .val = 2 },
            {.key = "on"         , .val = 9 },
            {.key = "one"        , .val = 1 },
            {.key = "only"       , .val = 4 },
            {.key = "opposite"   , .val = 1 },
            {.key = "options."   , .val = 1 },
            {.key = "other"      , .val = 2 },
            {.key = "other,"     , .val = 1 },
            {.key = "other."     , .val = 1 },
            {.key = "our"        , .val = 1 },
            {.key = "out"        , .val = 1 },
            {.key = "over"       , .val = 2 },
            {.key = "owned"      , .val = 2 },
            {.key = "park"       , .val = 1 },
            {.key = "people"     , .val = 1 },
            {.key = "pretty"     , .val = 1 },
            {.key = "problem"    , .val = 1 },
            {.key = "quickly."   , .val = 1 },
            {.key = "ran"        , .val = 2 },
            {.key = "river"      , .val = 7 },
            {.key = "river,"     , .val = 2 },
            {.key = "river."     , .val = 2 },
            {.key = "river\""    , .val = 1 },
            {.key = "row"        , .val = 3 },
            {.key = "rowing"     , .val = 1 },
            {.key = "running"    , .val = 1 },
            {.key = "sad"        , .val = 1 },
            {.key = "said,"      , .val = 1 },
            {.key = "sat"        , .val = 2 },
            {.key = "saw"        , .val = 1 },
            {.key = "scream"     , .val = 1 },
            {.key = "see"        , .val = 10},
            {.key = "seeing"     , .val = 1 },
            {.key = "she"        , .val = 17},
            {.key = "shocked"    , .val = 1 },
            {.key = "shore"      , .val = 1 },
            {.key = "shouted"    , .val = 1 },
            {.key = "show"       , .val = 1 },
            {.key = "shut"       , .val = 1 },
            {.key = "side"       , .val = 1 },
            {.key = "sing"       , .val = 1 },
            {.key = "sing,"      , .val = 1 },
            {.key = "skip"       , .val = 2 },
            {.key = "slammed"    , .val = 1 },
            {.key = "sleep"      , .val = 3 },
            {.key = "slept"      , .val = 1 },
            {.key = "smile"      , .val = 1 },
            {.key = "smiled,"    , .val = 1 },
            {.key = "so"         , .val = 9 },
            {.key = "some"       , .val = 1 },
            {.key = "someone"    , .val = 1 },
            {.key = "soon"       , .val = 1 },
            {.key = "sort"       , .val = 1 },
            {.key = "speak"      , .val = 2 },
            {.key = "spoke"      , .val = 1 },
            {.key = "stand"      , .val = 1 },
            {.key = "started"    , .val = 1 },
            {.key = "storm"      , .val = 1 },
            {.key = "story"      , .val = 2 },
            {.key = "story."     , .val = 1 },
            {.key = "swept"      , .val = 1 },
            {.key = "tea"        , .val = 1 },
            {.key = "than"       , .val = 2 },
            {.key = "that"       , .val = 4 },
            {.key = "the"        , .val = 24},
            {.key = "there"      , .val = 2 },
            {.key = "they"       , .val = 4 },
            {.key = "things."    , .val = 1 },
            {.key = "thinking"   , .val = 1 },
            {.key = "this"       , .val = 2 },
            {.key = "thought"    , .val = 3 },
            {.key = "tied"       , .val = 1 },
            {.key = "time"       , .val = 1 },
            {.key = "to"         , .val = 37},
            {.key = "today,"     , .val = 1 },
            {.key = "told"       , .val = 1 },
            {.key = "too"        , .val = 1 },
            {.key = "took"       , .val = 1 },
            {.key = "torn"       , .val = 1 },
            {.key = "town"       , .val = 1 },
            {.key = "town,"      , .val = 1 },
            {.key = "treating"   , .val = 1 },
            {.key = "tried"      , .val = 1 },
            {.key = "try"        , .val = 1 },
            {.key = "turned"     , .val = 1 },
            {.key = "up"         , .val = 3 },
            {.key = "upon"       , .val = 1 },
            {.key = "upset."     , .val = 1 },
            {.key = "very"       , .val = 4 },
            {.key = "waiting"    , .val = 1 },
            {.key = "walk"       , .val = 2 },
            {.key = "want"       , .val = 2 },
            {.key = "wanted"     , .val = 4 },
            {.key = "was"        , .val = 13},
            {.key = "wave"       , .val = 1 },
            {.key = "way"        , .val = 2 },
            {.key = "went"       , .val = 5 },
            {.key = "were"       , .val = 3 },
            {.key = "what"       , .val = 3 },
            {.key = "when"       , .val = 3 },
            {.key = "where"      , .val = 2 },
            {.key = "who"        , .val = 3 },
            {.key = "whole"      , .val = 1 },
            {.key = "why"        , .val = 1 },
            {.key = "will"       , .val = 2 },
            {.key = "with"       , .val = 10},
            {.key = "word"       , .val = 1 },
            {.key = "would"      , .val = 7 },
            {.key = "wrong"      , .val = 1 },
            {.key = "you"        , .val = 3 },
            {.key = "yourself\".", .val = 1 },
        }; 
        
        int a; int e; char *s;

        int all = 0;
        int success = 0;
        for(size_t i=0;i<308;i++) {
            s = data[i].key;
            e = data[i].val;
            a = map.items[hash_map_index(&map,s)].val;
            //
            all++;
            if (e==a) success++; else assertf(a == e, "expected word count = %d, but got %d for word [%s]", e, a, s);
        }

        inf("[\033[1;44m%-30s\033[0m] tests, all = %d --- success = %d", __func__, all, success);
        trc("misses on hash lookup = %lld", miss);
        trc("miss times on hash lookup = %lld", mizz);

    }

#   endif//__HASH_MAP_IMPL

#endif//__HASH_MAP_H