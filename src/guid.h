#pragma once
#define _GUID_H_IMPLEMENTATION

#ifndef _GUID_H
#define _GUID_H

#ifndef GUID_API
#define GUID_API
#endif//GUID_API

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
// #include "dates.h"
// #include "dyn_arr.h"
// #include "hash_map.h"
// #include "http_server.h"
// #include "inc/routes.h"
// #include "lists.h"
// #include "macros.h"
// #include "set.h"
// #include "string.h"
// #include "tokenizer.h"
#include "macros.h"
#include "time.h"


// -- INTERFACE
GUID_API void GUID_H_new(char* dest);
// -- IMPLEMENTATION



#ifdef  _GUID_H_IMPLEMENTATION



GUID_API void GUID_H_new(char* dest) {
    if(dest != NULL) memset(dest,0,strlen(dest));
    char guid   [] = "########-####-####-####-############";
    char digits [] = "07fca6bd3984e251";
    size_t m=strlen(digits);
    size_t l=strlen(guid);
    for(size_t i=0;i<l;++i){
        //printf("randome(%lld)=%lld\n",i,rand());
        if(guid[i]=='#') guid[i] = digits[rand()%m];
    }
    sprintf(dest,"%s",guid);
}
#endif//_GUID_H_IMPLEMENTATION


#endif//_GUID_H