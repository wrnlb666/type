#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>


typedef enum var_type {
    VAR_INT     = 'i',  // 64 bits integers
    VAR_UINT    = 'u',  // 64 bits unsigned integers
    VAR_FLOAT   = 'f',  // 64 bits double precision floating points
    VAR_STRING  = 's',  // C style NULL terminated strings
    VAR_PAIR    = 'p',  // key-val pair, dict if having an array of pairs
    VAR_ARRAY   = 'a',  // fix sized array, struct will be implemented as array
    VAR_LIST    = 'l',  // dynamic array for array and dict with key-val pair
} var_type_t;

typedef struct var var_t;


#endif  // __TYPE_H__

