#include "type.h"

typedef struct var_list var_list_t;

struct var {
    var_type_t  type;
    uint32_t    hash;
    union {
        // basic types
        int64_t     i;
        uint64_t    u;
        double      f;
        char*       s;

        // key-val pair, dict will be implemented as list of pairs
        struct {
            var_t*  kv;         // var_t[2]
        } p;

        // fix sized array. Struct will be implemented as fix sized array
        struct {
            uint64_t    len;
            var_t*      av;     // array values
        } *a;

        // array like list? 
        var_list_t*     l;
    } data;
};

typedef struct var_node var_node_t;
struct var_node {
    var_t           vars[8];
    var_node_t*     next;
};

struct var_list {
    uint64_t    len;
    var_node_t  lv;
};


size_t foo(void) {
    return sizeof (var_t);
}
