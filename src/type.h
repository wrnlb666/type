#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


typedef enum var_type {
    VAR_INT,        // 64 bits integers
    VAR_UINT,       // 64 bits unsigned integers
    VAR_FLOAT,      // 64 bits double precision floating points
    VAR_STRING,     // C style NULL terminated strings
    VAR_PAIR,       // key-val pair, dict if having an array of pairs
    VAR_ARRAY,      // fix sized array, struct will be implemented as array
    VAR_LIST,       // dynamic array for array and dict with key-val pair
} var_type_t;

typedef struct var var_t;


#endif  // __TYPE_H__
