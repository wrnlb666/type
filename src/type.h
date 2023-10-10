#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>


typedef enum var_type {
    VAR_INT     = 'i',  // 64 bits integers
    VAR_UINT    = 'u',  // 64 bits unsigned integers
    VAR_FLOAT   = 'f',  // 64 bits double precision floating points
    VAR_STRING  = 's',  // sized string with NULL terminator. Can be used to represent binary data. 
    VAR_ARRAY   = 'a',  // fix sized array, struct will be implemented as array, random access: O(1)
    VAR_LIST    = 'l',  // dynamic array, random access time would be O(n)
    VAR_DICT    = 'd',  // dict
} var_type_t;

typedef struct var var_t;



// functions: 

var_t*  var_new(var_type_t t, ...);
var_t*  var_new_int(int64_t i);
var_t*  var_new_uint(uint64_t u);
var_t*  var_new_float(double f);
var_t*  var_new_string(const char* s, ...);
var_t*  var_new_array(var_t* var, ...);
var_t*  var_new_array_size(size_t size);
var_t*  var_new_list(var_t* var, ...);
var_t*  var_new_list_empty(void);
var_t*  var_new_dict(var_t* key_arr, var_t* val_arr);
void    var_delete(var_t* var);
bool    var_hash(const var_t* var, uint64_t* hash);
void    var_get(const var_t* var, const char* format, ...);
void    var_vget(const var_t* var, const char* format, va_list ap);
void    var_set(var_t* var, const char* format, ...);
void    var_vset(var_t* var, const char* format, va_list ap);

#endif  // __TYPE_H__

