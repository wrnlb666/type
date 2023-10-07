#ifndef __VARSTRUCT_H__
#define __VARSTRUCT_H__

#include "type.h"

typedef struct var_array    var_array_t;
typedef struct var_list     var_list_t;
typedef struct var_dict     var_dict_t;

struct var {
    var_type_t  type;
    uint32_t    ref;
    union {
        // basic types
        int64_t     i;
        uint64_t    u;
        double      f;
        char*       s;

        // fix sized array. Struct will be implemented as fix sized array
        var_array_t*    a;

        // array like list? 
        var_list_t*     l;

        // dict 
        var_dict_t*     d;
        // TODO: memory representation of dict
    } data;
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

