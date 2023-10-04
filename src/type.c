#include "type.h"


struct var {
    var_type_t  type;
    union {
        // basic types
        int64_t     i;
        uint64_t    u;
        double      f;
        char*       s;

        // key-val pair, dict will be implemented as array of pairs
        struct {
            var_t*  key;
            var_t*  val;
        } p;

        // array like list? Struct will also be implemented as list
        struct {
            uint32_t    len;
            uint32_t    cap;
            var_t*      vars;
        } l;
    } data;
};



