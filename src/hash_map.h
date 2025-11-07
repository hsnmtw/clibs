#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "macros.h"

#pragma once

#define MAP_INCREMENT_SIZE 1000


#ifndef __HASH_MAP_H
#define __HASH_MAP_H

#ifndef HASH_MAP_API
#define HASH_MAP_API
#endif//HASH_MAP_API

//
// --------------------------------------------------------------------
//    INTERFACE
// --------------------------------------------------------------------
//
    typedef struct entry_t {
        char *key;
        char *val;
        int  freq;
    } entry_t;

    typedef struct hash_map_t {
        entry_t *items;
        size_t count;
        size_t capacity;
    } hash_map_t;


    // perform a hash of a key/text gets a number
    HASH_MAP_API int  hash_map_hash(const char *text);

    // if the key == NULL, or empty string, return INT_MIN
    // attempts to locate the index of the given key if available in the map, 
    // if the map is full and key is not available (count==capacity), return -ve index (negative)
    // if key is unavailable, returns newly empty item index
    HASH_MAP_API int  hash_map_index(hash_map_t *map, const char *key);

    // determines if the key providd at given index is a match, or the entry is empty
    HASH_MAP_API int  hash_map_is_match_or_empty(hash_map_t *map, const char *key, int index);
    HASH_MAP_API int  hash_map_is_key_used(hash_map_t *map, const char *key, int index);

    // if the entries set is NULL or count == 0 or capacity == 0, resize, and make capacity doubled
    // after every add, the count is incremented
    // side effects: 
    //      - map items memory allocation is modified
    //      - capacity is incremented
    //      - count is incremented
    HASH_MAP_API void hash_map_add(hash_map_t *map, const char *key, const char *val);

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
    
    HASH_MAP_API char* hash_map_get(hash_map_t *map, const char *key);

    HASH_MAP_API void test_hash_map_hash(void);

//
// --------------------------------------------------------------------
//    IMPLEMENTATION
// --------------------------------------------------------------------
//

#   ifndef __HASH_MAP_IMPL
#   define __HASH_MAP_IMPL

    HASH_MAP_API int hash_map_hash(const char *text) {
        if (text == NULL) return INT_MIN;
        size_t n = strlen(text);
        int sum = 0;
        for(size_t i=0;i<n;++i) {
            sum += text[i]*7 + sum*359;
        }
        return sum;
    }

    HASH_MAP_API void hash_map_add(hash_map_t *map, const char *key, const char *val) {
        assertf(map != NULL, " hash map is null ");
        assertf(key != NULL || strcmp(key, "")==0, " the key cannot be NULL or empty ");
        //todo("%s",__func__);
        if (map->items == NULL || map->items == 0 || map->capacity < 1 || map->count + 1 >= map->capacity) {
            map->capacity += MAP_INCREMENT_SIZE;
            entry_t *items = (entry_t*)malloc(sizeof(entry_t)*map->capacity);
            memset(items,0,(sizeof(entry_t))*map->capacity);
            for(size_t i=0;i<map->count;++i) {
                items[i] = map->items[i];
            }
            free(map->items);
            map->items = items;
        }

        int index = hash_map_index(map, key);
        if (hash_map_is_key_used(map,key,index)) return;
        assertf (map->capacity > 0, "map capacity must be more than zero");
        assertf (index >=0 && index < (int)map->capacity, " index must be between [%d,%d] , the given value was %d", 0, (int)map->capacity, index);
        map->items[index] = (entry_t) {strdup(key),strdup(val),1};
        map->count += 1;
        //wrn("count = %zu", map->count);
    }

    HASH_MAP_API int  hash_map_is_match_or_empty(hash_map_t *map, const char *key, int index) {
        assertf(map != NULL, " hash map is null ");
        assertf(map->capacity > 0, " the hash map is not initialized yet ");
        assertf(index < (int)map->capacity && index >= 0," expect the index to be in range [0, %lld], the provided index is %d", map->capacity - 1, index);
        
        return map->items[index].key == NULL 
            || strcmp(key, map->items[index].key)==0;
    }

    HASH_MAP_API int  hash_map_is_key_used(hash_map_t *map, const char *key, int index) {
        assertf(map != NULL, " hash map is null ");
        assertf(map->capacity > 0, " the hash map is not initialized yet ");
        assertf(index < (int)map->capacity && index >= 0," expect the index to be in range [0, %lld], the provided index is %d", map->capacity - 1, index);
        
        return map->items[index].key != NULL && strcmp(key, map->items[index].key)!=0;
    }
    
    HASH_MAP_API int  hash_map_index(hash_map_t *map, const char *key) {
        assertf(map != NULL, " hash map is null ");
        if (map->capacity < 1) return INT_MIN;
        if (key == NULL || strcmp(key, "") == 0) return INT_MIN;
        
        assertf(map->count <= map->capacity, " map->count IS MORE THAN map->capacity ");

        int hash = hash_map_hash(key);
        int index = hash % map->capacity;

        if (hash_map_is_match_or_empty(map,key,index)) return index;

        int collide = index;
        //inf("capacity=%lld, collide=%d, key at [%d]='%s' attempt to find index for = '%s', count='%lld', capacity='%lld'", map->capacity, collide,index,map->items[index].key, key, map->count, map->capacity);
        while ( hash_map_is_key_used(map, key, index) ) {
            //wrn("try index = %d, looking for= '%s', key='%s'", index, key, map->items[index].key);
            index = (index + 1) % map->capacity;
            if (collide == index) {
                assertf(false, "[give up] we should not get to this point [%d]", collide);
                break; // give up
            }
        }
        return index;
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
            map->items[index].val = NULL;
            map->count--;
        }
    }

    HASH_MAP_API void hash_map_clear(hash_map_t *map) {
        if (map->capacity > 0 && map->count > 0) free(map->items);
        map->count = 0;
        map->capacity = 0;
    }

    HASH_MAP_API char *hash_map_get(hash_map_t *map, const char *key) {
        int index = hash_map_index(map,key);
        if (index < 0 || index >= (int)map->capacity) return NULL;
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

        inf("number of words = %lld", spaces+1);

        assertf(spaces == 683, "number of spaces must be 683, but got = %lld", spaces);

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
            char *val = hash_map_get(&map, key);
            if (val == NULL) {
                hash_map_add(&map,key,key);
            } else {
                int index = hash_map_index(&map, key);
                //wrn("key=%s -- index=%d",key,index);
                map.items[index].freq++;
            }
        }

        inf("number of items: %zu", map.count);
        FILE *fout = fopen("./tale.txt","w");
        for(size_t i=0;i < map.capacity;++i) {
            if (map.items[i].key == NULL) continue;
            char line[100] = {0};
            //sprintf(line, "`%s`,\n", map.items[i].key);
            //sprintf(line,"key = '%s', freq=%d, index='%lld'\n", map.items[i].key, map.items[i].freq, i);
            fwrite(line,sizeof(char),strlen(line),fout);
        }
        fclose(fout);
        assertf(map.count==308,"we should have 308 items, but got %lld", map.count);


        const entry_t data[] = {
            {.key = "1After"     , .freq = 1 },
            {.key = "Abi"        , .freq = 14},
            {.key = "Abi,"       , .freq = 2 },
            {.key = "Abigail"    , .freq = 3 },
            {.key = "Abigail's"  , .freq = 5 },
            {.key = "Abigail."   , .freq = 1 },
            {.key = "And"        , .freq = 1 },
            {.key = "As"         , .freq = 1 },
            {.key = "Aww"        , .freq = 1 },
            {.key = "Bob"        , .freq = 3 },
            {.key = "Bob's"      , .freq = 1 },
            {.key = "Bob,"       , .freq = 1 },
            {.key = "Everyday"   , .freq = 1 },
            {.key = "He"         , .freq = 1 },
            {.key = "I"          , .freq = 1 },
            {.key = "Not"        , .freq = 1 },
            {.key = "Of"         , .freq = 1 },
            {.key = "Once"       , .freq = 1 },
            {.key = "One"        , .freq = 1 },
            {.key = "She"        , .freq = 7 },
            {.key = "Sinbad!\""  , .freq = 1 },
            {.key = "Sinbad"     , .freq = 5 },
            {.key = "Sinbad,"    , .freq = 1 },
            {.key = "Sinbad."    , .freq = 3 },
            {.key = "So"         , .freq = 1 },
            {.key = "Tale"       , .freq = 1 },
            {.key = "Tears"      , .freq = 1 },
            {.key = "The"        , .freq = 1 },
            {.key = "Then"       , .freq = 2 },
            {.key = "They"       , .freq = 1 },
            {.key = "Tom"        , .freq = 7 },
            {.key = "Tom's"      , .freq = 2 },
            {.key = "Tom,"       , .freq = 1 },
            {.key = "Tom."       , .freq = 2 },
            {.key = "Tom\"."     , .freq = 1 },
            {.key = "\"I"        , .freq = 2 },
            {.key = "\"Please"   , .freq = 1 },
            {.key = "\"Please\"" , .freq = 1 },
            {.key = "\"WHAT!\""  , .freq = 1 },
            {.key = "\"well\""   , .freq = 1 },
            {.key = "\"you"      , .freq = 2 },
            {.key = "a"          , .freq = 9 },
            {.key = "about"      , .freq = 2 },
            {.key = "across"     , .freq = 6 },
            {.key = "acted."     , .freq = 1 },
            {.key = "advice"     , .freq = 1 },
            {.key = "advice,"    , .freq = 2 },
            {.key = "advice."    , .freq = 1 },
            {.key = "after"      , .freq = 1 },
            {.key = "again."     , .freq = 1 },
            {.key = "also"       , .freq = 1 },
            {.key = "and"        , .freq = 33},
            {.key = "angry"      , .freq = 1 },
            {.key = "around"     , .freq = 3 },
            {.key = "arrived"    , .freq = 1 },
            {.key = "as"         , .freq = 3 },
            {.key = "ask"        , .freq = 2 },
            {.key = "at"         , .freq = 2 },
            {.key = "away"       , .freq = 1 },
            {.key = "badly,"     , .freq = 1 },
            {.key = "badly."     , .freq = 1 },
            {.key = "banged"     , .freq = 1 },
            {.key = "banks"      , .freq = 1 },
            {.key = "be"         , .freq = 2 },
            {.key = "beat"       , .freq = 1 },
            {.key = "before"     , .freq = 1 },
            {.key = "begged"     , .freq = 1 },
            {.key = "best"       , .freq = 1 },
            {.key = "between"    , .freq = 1 },
            {.key = "boat"       , .freq = 3 },
            {.key = "both"       , .freq = 1 },
            {.key = "boyfriend"  , .freq = 1 },
            {.key = "boyfriend," , .freq = 2 },
            {.key = "boys"       , .freq = 1 },
            {.key = "bridge"     , .freq = 4 },
            {.key = "bridge,"    , .freq = 1 },
            {.key = "built,"     , .freq = 1 },
            {.key = "but"        , .freq = 6 },
            {.key = "called"     , .freq = 2 },
            {.key = "can"        , .freq = 1 },
            {.key = "cheated"    , .freq = 1 },
            {.key = "cheating"   , .freq = 1 },
            {.key = "confused"   , .freq = 1 },
            {.key = "continued"  , .freq = 1 },
            {.key = "could"      , .freq = 1 },
            {.key = "couldn't"   , .freq = 2 },
            {.key = "course"     , .freq = 1 },
            {.key = "crazy."     , .freq = 1 },
            {.key = "cried"      , .freq = 4 },
            {.key = "cried."     , .freq = 1 },
            {.key = "cross"      , .freq = 2 },
            {.key = "crossing"   , .freq = 1 },
            {.key = "crying."    , .freq = 1 },
            {.key = "cup"        , .freq = 1 },
            {.key = "days"       , .freq = 1 },
            {.key = "decide"     , .freq = 1 },
            {.key = "decided"    , .freq = 1 },
            {.key = "decision"   , .freq = 1 },
            {.key = "did"        , .freq = 1 },
            {.key = "didn't!"    , .freq = 1 },
            {.key = "didn't"     , .freq = 3 },
            {.key = "disgusted"  , .freq = 2 },
            {.key = "do,"        , .freq = 1 },
            {.key = "do."        , .freq = 1 },
            {.key = "door"       , .freq = 2 },
            {.key = "down"       , .freq = 1 },
            {.key = "each"       , .freq = 2 },
            {.key = "ends."      , .freq = 1 },
            {.key = "ever"       , .freq = 1 },
            {.key = "ever..."    , .freq = 1 },
            {.key = "everyone"   , .freq = 1 },
            {.key = "everything" , .freq = 2 },
            {.key = "explain"    , .freq = 1 },
            {.key = "explained"  , .freq = 4 },
            {.key = "face,"      , .freq = 1 },
            {.key = "fact"       , .freq = 1 },
            {.key = "felt"       , .freq = 2 },
            {.key = "few"        , .freq = 1 },
            {.key = "finished"   , .freq = 1 },
            {.key = "flooded"    , .freq = 1 },
            {.key = "for"        , .freq = 6 },
            {.key = "friend,"    , .freq = 1 },
            {.key = "frightful"  , .freq = 1 },
            {.key = "gave"       , .freq = 1 },
            {.key = "girl"       , .freq = 1 },
            {.key = "go"         , .freq = 1 },
            {.key = "great"      , .freq = 1 },
            {.key = "had"        , .freq = 4 },
            {.key = "hands"      , .freq = 1 },
            {.key = "hands."     , .freq = 1 },
            {.key = "happened"   , .freq = 2 },
            {.key = "happy"      , .freq = 1 },
            {.key = "he"         , .freq = 8 },
            {.key = "help"       , .freq = 1 },
            {.key = "help."      , .freq = 1 },
            {.key = "her"        , .freq = 18},
            {.key = "her,"       , .freq = 2 },
            {.key = "herself."   , .freq = 1 },
            {.key = "him"        , .freq = 2 },
            {.key = "him,"       , .freq = 2 },
            {.key = "him."       , .freq = 2 },
            {.key = "his"        , .freq = 4 },
            {.key = "hold"       , .freq = 1 },
            {.key = "holding"    , .freq = 1 },
            {.key = "house"      , .freq = 4 },
            {.key = "how"        , .freq = 2 },
            {.key = "if"         , .freq = 1 },
            {.key = "in"         , .freq = 5 },
            {.key = "into"       , .freq = 2 },
            {.key = "is"         , .freq = 1 },
            {.key = "it"         , .freq = 1 },
            {.key = "it,"        , .freq = 1 },
            {.key = "it."        , .freq = 1 },
            {.key = "jumped"     , .freq = 1 },
            {.key = "kept"       , .freq = 1 },
            {.key = "knew"       , .freq = 1 },
            {.key = "know"       , .freq = 1 },
            {.key = "known"      , .freq = 1 },
            {.key = "least"      , .freq = 1 },
            {.key = "left"       , .freq = 1 },
            {.key = "let"        , .freq = 1 },
            {.key = "listened"   , .freq = 1 },
            {.key = "little"     , .freq = 2 },
            {.key = "lived"      , .freq = 1 },
            {.key = "lived."     , .freq = 1 },
            {.key = "looked"     , .freq = 1 },
            {.key = "looking"    , .freq = 1 },
            {.key = "love"       , .freq = 1 },
            {.key = "love."      , .freq = 1 },
            {.key = "loved"      , .freq = 1 },
            {.key = "lovely!"    , .freq = 1 },
            {.key = "lovely."    , .freq = 1 },
            {.key = "make"       , .freq = 1 },
            {.key = "man"        , .freq = 2 },
            {.key = "many"       , .freq = 1 },
            {.key = "me"         , .freq = 1 },
            {.key = "me..."      , .freq = 1 },
            {.key = "me.\""      , .freq = 1 },
            {.key = "months"     , .freq = 1 },
            {.key = "more"       , .freq = 2 },
            {.key = "more,"      , .freq = 1 },
            {.key = "morning"    , .freq = 1 },
            {.key = "mother"     , .freq = 5 },
            {.key = "mother,"    , .freq = 1 },
            {.key = "mother."    , .freq = 1 },
            {.key = "much"       , .freq = 2 },
            {.key = "my"         , .freq = 1 },
            {.key = "need"       , .freq = 1 },
            {.key = "needed"     , .freq = 1 },
            {.key = "new"        , .freq = 1 },
            {.key = "next"       , .freq = 1 },
            {.key = "nice"       , .freq = 1 },
            {.key = "night"      , .freq = 1 },
            {.key = "no"         , .freq = 2 },
            {.key = "now"        , .freq = 2 },
            {.key = "of"         , .freq = 3 },
            {.key = "off"        , .freq = 1 },
            {.key = "offered"    , .freq = 2 },
            {.key = "on"         , .freq = 9 },
            {.key = "one"        , .freq = 1 },
            {.key = "only"       , .freq = 4 },
            {.key = "opposite"   , .freq = 1 },
            {.key = "options."   , .freq = 1 },
            {.key = "other"      , .freq = 2 },
            {.key = "other,"     , .freq = 1 },
            {.key = "other."     , .freq = 1 },
            {.key = "our"        , .freq = 1 },
            {.key = "out"        , .freq = 1 },
            {.key = "over"       , .freq = 2 },
            {.key = "owned"      , .freq = 2 },
            {.key = "park"       , .freq = 1 },
            {.key = "people"     , .freq = 1 },
            {.key = "pretty"     , .freq = 1 },
            {.key = "problem"    , .freq = 1 },
            {.key = "quickly."   , .freq = 1 },
            {.key = "ran"        , .freq = 2 },
            {.key = "river"      , .freq = 7 },
            {.key = "river,"     , .freq = 2 },
            {.key = "river."     , .freq = 2 },
            {.key = "river\""    , .freq = 1 },
            {.key = "row"        , .freq = 3 },
            {.key = "rowing"     , .freq = 1 },
            {.key = "running"    , .freq = 1 },
            {.key = "sad"        , .freq = 1 },
            {.key = "said,"      , .freq = 1 },
            {.key = "sat"        , .freq = 2 },
            {.key = "saw"        , .freq = 1 },
            {.key = "scream"     , .freq = 1 },
            {.key = "see"        , .freq = 10},
            {.key = "seeing"     , .freq = 1 },
            {.key = "she"        , .freq = 17},
            {.key = "shocked"    , .freq = 1 },
            {.key = "shore"      , .freq = 1 },
            {.key = "shouted"    , .freq = 1 },
            {.key = "show"       , .freq = 1 },
            {.key = "shut"       , .freq = 1 },
            {.key = "side"       , .freq = 1 },
            {.key = "sing"       , .freq = 1 },
            {.key = "sing,"      , .freq = 1 },
            {.key = "skip"       , .freq = 2 },
            {.key = "slammed"    , .freq = 1 },
            {.key = "sleep"      , .freq = 3 },
            {.key = "slept"      , .freq = 1 },
            {.key = "smile"      , .freq = 1 },
            {.key = "smiled,"    , .freq = 1 },
            {.key = "so"         , .freq = 9 },
            {.key = "some"       , .freq = 1 },
            {.key = "someone"    , .freq = 1 },
            {.key = "soon"       , .freq = 1 },
            {.key = "sort"       , .freq = 1 },
            {.key = "speak"      , .freq = 2 },
            {.key = "spoke"      , .freq = 1 },
            {.key = "stand"      , .freq = 1 },
            {.key = "started"    , .freq = 1 },
            {.key = "storm"      , .freq = 1 },
            {.key = "story"      , .freq = 2 },
            {.key = "story."     , .freq = 1 },
            {.key = "swept"      , .freq = 1 },
            {.key = "tea"        , .freq = 1 },
            {.key = "than"       , .freq = 2 },
            {.key = "that"       , .freq = 4 },
            {.key = "the"        , .freq = 24},
            {.key = "there"      , .freq = 2 },
            {.key = "they"       , .freq = 4 },
            {.key = "things."    , .freq = 1 },
            {.key = "thinking"   , .freq = 1 },
            {.key = "this"       , .freq = 2 },
            {.key = "thought"    , .freq = 3 },
            {.key = "tied"       , .freq = 1 },
            {.key = "time"       , .freq = 1 },
            {.key = "to"         , .freq = 37},
            {.key = "today,"     , .freq = 1 },
            {.key = "told"       , .freq = 1 },
            {.key = "too"        , .freq = 1 },
            {.key = "took"       , .freq = 1 },
            {.key = "torn"       , .freq = 1 },
            {.key = "town"       , .freq = 1 },
            {.key = "town,"      , .freq = 1 },
            {.key = "treating"   , .freq = 1 },
            {.key = "tried"      , .freq = 1 },
            {.key = "try"        , .freq = 1 },
            {.key = "turned"     , .freq = 1 },
            {.key = "up"         , .freq = 3 },
            {.key = "upon"       , .freq = 1 },
            {.key = "upset."     , .freq = 1 },
            {.key = "very"       , .freq = 4 },
            {.key = "waiting"    , .freq = 1 },
            {.key = "walk"       , .freq = 2 },
            {.key = "want"       , .freq = 2 },
            {.key = "wanted"     , .freq = 4 },
            {.key = "was"        , .freq = 13},
            {.key = "wave"       , .freq = 1 },
            {.key = "way"        , .freq = 2 },
            {.key = "went"       , .freq = 5 },
            {.key = "were"       , .freq = 3 },
            {.key = "what"       , .freq = 3 },
            {.key = "when"       , .freq = 3 },
            {.key = "where"      , .freq = 2 },
            {.key = "who"        , .freq = 3 },
            {.key = "whole"      , .freq = 1 },
            {.key = "why"        , .freq = 1 },
            {.key = "will"       , .freq = 2 },
            {.key = "with"       , .freq = 10},
            {.key = "word"       , .freq = 1 },
            {.key = "would"      , .freq = 7 },
            {.key = "wrong"      , .freq = 1 },
            {.key = "you"        , .freq = 3 },
            {.key = "yourself\".", .freq = 1 },
        }; 
        
        int a; int e; char *s;

        int all = 0;
        int success = 0;
        for(size_t i=0;i<308;i++) {
            s = data[i].key;
            e = data[i].freq;
            a = map.items[hash_map_index(&map,s)].freq;
            //
            all++;
            if (e==a) success++; else assertf(a == e, "expected word count = %d, but got %d for word [%s]", e, a, s);
        }

        printf("[\033[1;44m%-30s\033[0m] tests, all = %d --- success = %d\n", __func__, all, success);

    }

#   endif//__HASH_MAP_IMPL

#endif//__HASH_MAP_H