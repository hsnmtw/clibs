#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "macros.h"

#pragma once

#define MAP_INCREMENT_SIZE 100*1000


#ifndef __HASH_MAP_H
#define __HASH_MAP_H

#ifndef HASH_MAP_API
#define HASH_MAP_API
#endif//HASH_MAP_API

static size_t miss = 0;
static size_t mizz = 0;
static size_t hits = 0;

//
// --------------------------------------------------------------------
//    INTERFACE
// --------------------------------------------------------------------
//
    typedef struct {
        char *key;
        int   val;
        bool  use;
    } entry_t;

    typedef struct {
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

    HASH_MAP_API void hash_map_init(hash_map_t *map);

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

    

    HASH_MAP_API uint32_t hash_map_hash(const char *text) {
        if (text == NULL) return INT_MIN;
        size_t n = strlen(text);
        uint32_t hash = MAP_INCREMENT_SIZE/3;
        for(size_t i=0;i<n;++i) {
            hash = (hash << 5) + (uint32_t)(text[i]-'a')*(1+(uint32_t)pow(i+1, (uint32_t)(text[i]-'a')));
        }
        return hash;
    }

    HASH_MAP_API void hash_map_init(hash_map_t *map) {
        size_t n = map->capacity;
        if (n<1) n = MAP_INCREMENT_SIZE;
        entry_t *items = (entry_t*)malloc(sizeof(entry_t)*n);
        memset(items,0,sizeof(entry_t)*n);
        if (map->items != NULL) {
            for(size_t i=0;i<n;++i) {
                if (map->items[i].use == false) continue;
                free(map->items[i].key);
            }
            memset(map->items,0,sizeof(entry_t)*n);
            free(map->items);
        }
        map->items = items;
        map->capacity = n;
        map->count = 0;
    }

    HASH_MAP_API void hash_map_add(hash_map_t *map, const char *key, const int val) {
        assertf(map != NULL, " hash map is null ");
        assertf(key != NULL || strcmp(key, "")==0, " the key cannot be NULL or empty ");
        //todo("%s",__func__);
        if (map->items == NULL              ) hash_map_init(map);
        if (map->count + 1 >= map->capacity && map->items != NULL) {            
            // inf("expand ... %zu -- %zu || %d", map->count, map->capacity, map->count + 1 >= map->capacity);            
            size_t n = map->capacity + MAP_INCREMENT_SIZE;
            entry_t *items = (entry_t*)malloc(sizeof(entry_t)*n);
            memset(items,0,sizeof(entry_t)*n);
            size_t c = 0;
            for(size_t i=0;i<map->capacity;++i) {
                entry_t e = map->items[i];
                if (e.use == false) continue;
                uint32_t index = hash_map_hash(e.key);
                while(items[index % n].use) err("should not collide '%s' / '%s'",items[index++ % n].key, e.key);
                items[index % n] = e;
                c++;
            }
            assert(c == map->count);
            // inf("count = %lld , c= %d", map->count, c);
            free(map->items);
            map->items = items;
            map->capacity = n;
            map->count = c;
        }

        int index = hash_map_index(map, key);
        
        assertf (index>-1 && index < (int)map->capacity,"index=%d , count = %zu, capacity = %zu", index, map->count, map->capacity)
        assertf (map->items[index].use == false || strcmp(map->items[index].key, key) == 0, "expect item at index %d to be NULL or of key = `%s`, but got '%s'", index, key, map->items[index].key );
        assertf (map->capacity > 0, "map capacity must be more than zero");
        assertf (map->count < map->capacity, "map count must be non-negative");
        // assertf (index >=0 && index < (int)map->capacity, " index must be between [%d,%d] , the given value was %d", 0, (int)map->capacity, index);
        map->items[index] = (entry_t) {strdup(key),val, true};
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
        hits++;
        assertf (map != NULL                         , " hash map is null ");
        assertf (map->items != NULL                  , " hash map items is null ");        
        assertf (map->capacity > 0                   , " capacity [%zu] is less than 1, count=%zu", map->capacity, map->count);
        assertf (key != NULL && strcmp(key, "") != 0 , " key must be NON-NULL and NOT EMPTY !");        
        assertf (map->count <= map->capacity         , " map->count IS MORE THAN map->capacity ");
        
        int n=map->capacity;
        int index = hash_map_hash(key) % n;
        if (map->items[index].use && strcmp(map->items[index].key,key)==0) return index;
        mizz++;
        for (int i=0;i<n && map->items[index].use && strcmp(map->items[index].key,key)!=0;++i) {
            index = (index+1)%n;
            miss++;
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

    #define BUFF_LEN 2048

    size_t read_file_to_map(hash_map_t *map, char *file_path) {
        size_t words = 0;
        FILE *file = fopen(file_path, "r");
        if (file == NULL) {
            err("[%s] failed to open file ... 1", file_path);
            fclose(file);
            return 1;
        }

        char buffer[BUFF_LEN] = {0};
        char word[100] = {0};
        
        int r;
        size_t i;
        //size_t w=1;
        // hash_map_t map = {0};
        for(i=0;i<100;++i) word[i] = 0;
        i=0;
        hash_map_init(map);
        while ((r = fread(buffer,1,BUFF_LEN,file)) > 0) {
            for(int b=0;b<r;++b){
                //inf("[%zu][%zu] ch='%c'",i,b,buffer[b]);
                bool is_last_word = r<BUFF_LEN && b==r-1;
                if (isspace(buffer[b]) || i>=100 || is_last_word) {
                    words++;
                    if (is_last_word) 
                        word[i++] = buffer[b];
                    if (strlen(word)>0) {
                        int index = hash_map_index(map,word);
                        if (!(map->items[index].use == false || strcmp(map->items[index].key, word) != 0)) {
                            map->items[index].val += 1;
                            //wrn(" <%s> : [%5s] '%s'", file_path,"",word);
                        } else {
                            //inf(" <%s> : [%5lld] '%s'", file_path,w++,word);
                            hash_map_add(map,word,1);
                            //fclose(file);
                            //return 0;
                        }
                    }
                    for(size_t j=0;j<100 && j<i;++j) word[j] = 0;
                    i=0;
                    continue;
                }
                char c = buffer[b];
                if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
                    word[i++] = buffer[b];
            }
        }
        fclose(file);

        return words;        
    }

    #define MAX_WORD_LENGTH 1023

    size_t scan_file_to_map(hash_map_t *map, char *file_path) {

        hash_map_init(map);

        FILE *file = fopen(file_path, "r");
        if (file == NULL) {
            err("[%s] failed to open file ... 1", file_path);
            fclose(file);
            return -1;
        }

        char word[MAX_WORD_LENGTH] = {0};
        size_t w = 1;
        size_t words = 0;
        
        while (fscanf(file, " %1023s", word) == 1) {
            if (strlen(word)>0) {
                int index = hash_map_index(map,word);
                if (!(map->items[index].use == false || strcmp(map->items[index].key, word) != 0)) {
                    map->items[index].val += 1;
                    //wrn(" <%s> : [%5s] '%s'", file_path,"",word);
                } else {
                    inf(" <%s> : [%5lld] '%s'", file_path,w++,word);
                    hash_map_add(map,word,1);
                }
                words++;
            }
        }

        fclose(file);     
        return words;   
    }

    HASH_MAP_API void test_hash_map_hash(void) {

        hash_map_t map = {0};
        scan_file_to_map(&map,"./testdata/AbigailsTale.txt");
        
        inf("number of items: %zu", map.count);
        FILE *fout = fopen("./testdata/tale.txt","w");
        for(size_t i=0;i < map.capacity;++i) {
            if (map.items[i].use == false) continue;
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
        trc("hits times on hash lookup = %lld", hits);
        int index = -1;
        int max = INT_MIN;
        for(size_t i=0;i<map.capacity;++i) {
            if (max<map.items[i].val) {
                max = map.items[i].val;
                index = i;
            }
        }
        inf("-----------------------------------");
        inf("max occured word is : '%s' = '%d'", map.items[index].key, map.items[index].val);        
    }


    #define BAR '|'

    HASH_MAP_API void test_hash_map_story(char *file_name_story, char *file_name_count) {

        
        hash_map_t map = {0};
        size_t words = read_file_to_map(&map,file_name_story);

        int max = INT_MIN;
        entry_t e = {0};
        for(size_t i=0;i<map.capacity;++i) {
            if (max<map.items[i].val) {
                max = map.items[i].val;
                e.key = strdup(map.items[i].key);
                e.val = map.items[i].val;
            }
        }
        inf("----------------------------------------");
        inf("max occured word is : '%s' = '%d'", e.key, e.val);


        inf("````````````````````````````````````````");
        inf("number of unique words: %zu", map.count);
        inf("number of words: %zu", words);
        inf("````````````````````````````````````````");
        FILE *fout = fopen(file_name_count,"w");
        for(size_t i=0;i < map.capacity;++i) {
            if (map.items[i].use == false) continue;
            char line[100] = {0};
            char bar[] = "          ";
            size_t percent = (size_t)(10*(float)map.items[i].val/(float)max);
            for(size_t i=0;i<sizeof(bar) && i<percent;++i) {
                bar[i]=BAR;
            }
            sprintf(line, "%-12s %6d %s\n", bar, map.items[i].val, map.items[i].key);
            //sprintf(line,"key = '%s', freq=%d, index='%zu'\n", map.items[i].key, map.items[i].freq, i);
            fwrite(line,sizeof(char),strlen(line),fout);
            if(map.items[i].use && map.items[i].key != NULL) free (map.items[i].key);            
        }
        free(map.items);
        fclose(fout);
        //assertf(map.count==67505,"we should have 67505 items, but got %zu", map.count);

        inf("[\033[1;44m%-30s\033[0m] tests", file_name_story);
        trc("misses on hash lookup = %lld", miss);
        trc("miss times on hash lookup = %lld", mizz);
        trc("hits times on hash lookup = %lld", hits);


    }

    HASH_MAP_API void test_hash_map_hash_shakespeare(void) {
        test_hash_map_story("./testdata/t8.shakespeare.txt","./testdata/t8.shakespeare.counts.txt");
    }
    HASH_MAP_API void test_hash_map_hash_houn(void) {
        test_hash_map_story("./testdata/houn.txt","./testdata/houn.counts.txt");
    }
    HASH_MAP_API void test_hash_map_hash_black_peter(void) {
        test_hash_map_story("./testdata/black-peter.txt","./testdata/black-peter.counts.txt");
    }
    

#   endif//__HASH_MAP_IMPL

#endif//__HASH_MAP_H