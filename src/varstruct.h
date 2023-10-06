#ifndef __VARSTRUCT_H__
#define __VARSTRUCT_H__

#include "type.h"

typedef struct var_pair     var_pair_t;
typedef struct var_array    var_array_t;
typedef struct var_list     var_list_t;

struct var {
    var_type_t  type;
    uint32_t    ref;
    union {
        // basic types
        int64_t     i;
        uint64_t    u;
        double      f;
        char*       s;

        // key-val pair, dict will be implemented as list of pairs
        var_pair_t*     p;
        
        // fix sized array. Struct will be implemented as fix sized array
        var_array_t*    a;

        // array like list? 
        var_list_t*     l;
    } data;
};

struct var_pair {
    var_t*      key;
    var_t*      val;
    uint64_t    hash;
};

struct var_array {
    uint64_t    len;
    var_t**     av;
};

#define LIST_SIZE 8
typedef struct var_node var_node_t;
struct var_node {
    var_t*          vars[LIST_SIZE];
    var_node_t*     next;
};

struct var_list {
    uint64_t    len;
    var_node_t  lv;
};


#endif  // __VARSTRUCT_H__

