#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifndef TOKENIZER_H_API
#define TOKENIZER_H_API
#endif//TOKENIZER_H_API
#include "macros.h"

typedef struct {
    char *string;
    size_t pos;
} tokenizer_t;

// INTERFACE
TOKENIZER_H_API char *tokenize_next(tokenizer_t *tokenizer);

// IMPLEMENTATION
TOKENIZER_H_API char *tokenize_next(tokenizer_t *tokenizer) {
    size_t l,i=tokenizer->pos,j=0;

    if (tokenizer == NULL || tokenizer->string == NULL || (l = strlen(tokenizer->string))==0 || i >= l) {
        return NULL;
    }
    
    while( i<l && isspace(tokenizer->string[i]) ) { 
        if (tokenizer->string[i] == '\n') {
            tokenizer->pos = i+1;
            return strdup("\n");
        }
        i++;
    }

    tokenizer->pos = i;
    
    if (i>=l) return NULL;
    
    char *token = (char*)malloc(l-i+1);
    memset(token,0,l-i+1);
    
    while( i<l && j < l-tokenizer->pos && !isspace(tokenizer->string[i]) ) {
        if (!isalnum(tokenizer->string[i])) {
            tokenizer->pos = i+1;
            return &tokenizer->string[i];
        }
        token[j++] = tokenizer->string[i++];
    }

    tokenizer->pos = i;

    return token;
}