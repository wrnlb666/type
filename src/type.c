#include "varstruct.h"

// error msgs
#define ERRO(msg) \
exit((fprintf(stderr, "[ERRO]: " msg "\n"), 1));

// check if allocation returns `NULL`
#define MEM_CHECK(ptr) \
if (ptr == NULL) ERRO("out of memory");


// constructor

/*
 * for array and list, varadic arguments should end with `NULL`
 * for array, list, and pair, arg type should be `var_t*`
 * constructor for array, list, and pair won't add ref to the existing `var_t*`
 * for string, format string is allowed
 * for rest of the types, it should be their corresponding C type. 
 *
 * @param   t   type of the `var_t`, can be char or enum 
 * @param   ... watch above description
 * @return      pointer to a new `var_t` that currently has one ref
 */
var_t* var_init(var_type_t t, ...) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);

    // start varadic arguments
    va_list ap;
    va_start(ap, t);

    switch (t) {
        case 'a': {
            // allocate struct memory
            res->data.a = malloc(sizeof (var_array_t));
            MEM_CHECK(res->data.a);
            
            // get argument length
            res->data.a->len = 0;
            for (var_t* temp = va_arg(ap, var_t*); 
                temp != NULL; 
                temp = (res->data.a->len++, va_arg(ap, var_t*)));
            va_end(ap);

            // allocate array memory
            res->data.a->av = malloc(sizeof (var_t) * res->data.a->len);
            
            // assign values
            va_start(ap, t);
            for (size_t i = 0; i < res->data.a->len; i++) {
                res->data.a->av[i] = va_arg(ap, var_t*);
            }
        }
        break;

        case 'd': {
            // TODO: dictionary requires struct and hash. 
        }
        break;
        
        case 'f': {
            res->data.f = va_arg(ap, double);
        }
        break;
        
        case 'i': {
            res->data.i = va_arg(ap, int64_t);
        }
        break;

        case 'l': {
            // allocate struct memory 
            res->data.l = malloc(sizeof (var_list_t));
            MEM_CHECK(res->data.l);
            
            // get argument length
            res->data.l->len = 0;
            for (var_t* temp = va_arg(ap, var_t*); 
                temp != NULL; 
                temp = (res->data.l->len++, va_arg(ap, var_t*)));
            va_end(ap);    
            va_start(ap, t);

            // early return if len is 0 
            if (res->data.l->len == 0) return res;

            // TODO: calculate total needed nodes
            size_t node_count = (res->data.l->len + LIST_SIZE - 1) / LIST_SIZE;
            
            // allocate memory
            var_node_t* curr = &(res->data.l->lv);
            for (size_t i = 1; i < node_count; i++) {
                curr->next = malloc(sizeof (var_node_t));
                MEM_CHECK(curr->next);
                curr = curr->next;
            }
            curr->next = NULL;

            // assign values 
            curr = &(res->data.l->lv);
            for (size_t i = 0; i < res->data.l->len; i += LIST_SIZE) {
                for (size_t j = 0; j < LIST_SIZE && i + j < res->data.l->len; j++) {
                    curr->vars[i] = va_arg(ap, var_t*);
                }
                curr = curr->next;
            }
        }
        break;

        case 's': {
            char* str = va_arg(ap, char*);
            int str_len = vsnprintf(NULL, 0, str, ap);
            va_end(ap);
            if (str_len < 0) ERRO("invalid format string");

            res->data.s = malloc(str_len + 1);
            MEM_CHECK(res->data.s);

            va_start(ap, t);
            va_arg(ap, char*);
            vsprintf(res->data.s, str, ap);
        }
        break;

        case 'u': {
            res->data.u = va_arg(ap, uint64_t);
        }
        break;

        default: {
            va_end(ap);
            ERRO("unknown type");
        }
    }

    // cleanup varadic arguments
    va_end(ap);

    // set type and ref
    res->type   = t;
    res->ref    = 1;

    return res;
}


var_t* var_new_int(int64_t i) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->data.i = i;
    res->ref = 1;
    return res;
}


var_t* var_new_uint(uint64_t u) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->data.u = u;
    res->ref = 1;
    return res;
}

size_t foo(void) {
    return sizeof (var_t);
}
