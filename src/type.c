#include "type.h"


struct var {
    var_type_t  type;
    uint64_t    hash;
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
            uint32_t    len;
            var_t*      av;     // array values
        } a;

        // array like list? 
        struct {
            uint32_t    len;
            struct node {
                var_t*          vars;       // var_t[8] for now. 
                struct node*    next;
            } *lv;              // list values
        } l;
    } data;
};


size_t foo(void) {
    return sizeof (var_t);
}
