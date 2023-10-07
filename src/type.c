#include "varstruct.h"

// error msgs
#define ERRO(msg) \
exit((fprintf(stderr, "[ERRO]: " msg "\n"), 1));

// check if allocation returns `NULL`
#define MEM_CHECK(ptr) \
if (ptr == NULL) ERRO("out of memory");


// constructor

/*
 * for array, list, and dict, varadic arguments should end with `NULL`
 * for array, list, and dict, arg type should be `var_t*`
 * for string, format string is allowed
 * for rest of the types, it should be their corresponding C type. 
 *
 * @param   t   type of the `var_t`, can be char or enum 
 * @param   ... watch above description
 * @return      pointer to a new `var_t`
 */
var_t* var_init(var_type_t t, ...) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);

    // start varadic arguments
    va_list ap;
    va_start(ap, t);

    switch (t) {
        case 'a': {
            // get argument length 
            size_t arr_len = 0;
            for (var_t* temp = va_arg(ap, var_t*); 
                temp != NULL; 
                temp = (arr_len++, va_arg(ap, var_t*)));
            va_end(ap);

            // allocate struct memory
            res->data.a = malloc(sizeof (var_array_t) + sizeof (var_t*) * arr_len);
            MEM_CHECK(res->data.a);
            
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

            // calculate total needed nodes
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
                    curr->vars[j] = va_arg(ap, var_t*);
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

            res->data.s = malloc(sizeof (var_string_t) + str_len + 1);
            MEM_CHECK(res->data.s);

            res->data.s->len = str_len;
            va_start(ap, t);
            va_arg(ap, char*);
            vsprintf(res->data.s->str, str, ap);
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

    return res;
}


var_t* var_new_int(int64_t i) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->data.i = i;
    return res;
}


var_t* var_new_uint(uint64_t u) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->data.u = u;
    return res;
}


var_t* var_new_float(double f) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->data.f = f;
    return res;
}

var_t* var_new_string(const char* s, ...) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);

    va_list ap;
    va_start(ap, s);
    int str_len = vsnprintf(NULL, 0, s, ap);
    va_end(ap);
    if (str_len < 0) ERRO("invalid format string");

    res->data.s = malloc(sizeof (var_string_t) + str_len + 1);
    MEM_CHECK(res->data.s);

    res->data.s->len = str_len;
    va_start(ap, s);
    vsprintf(res->data.s->str, s, ap);
    va_end(ap);

    return res;
}


var_t* var_new_array(size_t size) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->data.a = malloc(sizeof (var_array_t) + sizeof (var_t*) * size);
    MEM_CHECK(res->data.a);
    res->data.a->len = size;
    return res;
}


/*
 * @param   var first element of the list
 * @param   ... the rest of the elements, should end with `NULL`
 * @return      a new `var_t*` of type `VAR_LIST`
 */
var_t* var_new_list(var_t* var, ...) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);

    va_list ap;
    va_start(ap, var);

    // allocate struct memory 
    res->data.l = malloc(sizeof (var_list_t));
    MEM_CHECK(res->data.l);
    
    // get argument length
    res->data.l->len = 1;
    for (var_t* temp = va_arg(ap, var_t*); 
        temp != NULL; 
        temp = (res->data.l->len++, va_arg(ap, var_t*)));
    va_end(ap);    
    va_start(ap, var);

    // calculate total needed nodes
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
    curr->vars[0] = var;
    for (size_t i = 1; i < LIST_SIZE && i < res->data.l->len; i++) {
        curr->vars[i] = va_arg(ap, var_t*);
    }
    curr = curr->next;
    for (size_t i = 1; i < res->data.l->len; i += LIST_SIZE) {
        for (size_t j = 0; j < LIST_SIZE && i + j < res->data.l->len; j++) {
            curr->vars[j] = va_arg(ap, var_t*);
        }
        curr = curr->next;
    }

    va_end(ap);

    return res;
}


var_t* var_new_list_empty(void) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->data.l = malloc(sizeof (var_list_t));
    MEM_CHECK(res->data.l);
    res->data.l->len = 0;
    res->data.l->lv.next = NULL;
    return res;
}


var_t* var_new_dict(void) {
    // TODO: implement dict
    
    return NULL;
}

size_t foo(void) {
    return sizeof (var_t);
}
