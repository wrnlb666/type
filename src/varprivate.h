#ifndef __VARSTRUCT_H__
#define __VARSTRUCT_H__

#include "type.h"

#ifdef TYPE_GC
#include <gc.h>
#define malloc(size)        GC_malloc(size)
#define calloc(nmemb, size) GC_malloc(nmemb * size)
#define realloc(ptr, size)  GC_realloc(ptr, size)
#define free(ptr)           GC_free(ptr)
#endif  // TYPE_GC

typedef struct var_string   var_string_t;
typedef struct var_array    var_array_t;
typedef struct var_list     var_list_t;
typedef struct var_dict     var_dict_t;

struct var {
    var_type_t  type;
    union {
        // basic types
        int64_t     i;
        uint64_t    u;
        double      f;
        
        // sized string 
        var_string_t*   s;

        // fix sized array. Struct will be implemented as fix sized array
        var_array_t*    a;

        // array like list? 
        var_list_t*     l;

        // dict 
        var_dict_t*     d;
        // TODO: memory representation of dict
    } data;
};

// structs fo string 
struct var_string {
    uint32_t    len;
    char        str[];
};

// structs for array
struct var_array {
    uint64_t    len;
    var_t*      av[];
};

// structs for list
#define LIST_SIZE 16
typedef struct var_node var_node_t;
struct var_node {
    var_t*          vars[LIST_SIZE];
    var_node_t*     next;
};

struct var_list {
    uint64_t    len;
    var_node_t  lv;
};

// structs for dict
#define DICT_SIZE   16
#define DICT_HASH   0xcbf29ce484222325LLU
#define DICT_PRIME  0x100000001b3LLU
#define DICT_RATIO  0x9e3779b97f4a7c15LLU
typedef struct var_dict_elem var_dict_elem_t;
struct var_dict_elem {
    uint64_t            hash;
    var_t*              key;
    var_t*              val;
    var_dict_elem_t*    prev;
    var_dict_elem_t*    next;
};

typedef struct var_dict_list var_dict_list_t;
struct var_dict_list {
    size_t              size;
    var_dict_elem_t*    head;
    var_dict_elem_t*    tail;
};

struct var_dict {
    uint64_t            mod;
    var_dict_list_t*    list;
};


#endif  // __VARSTRUCT_H__

