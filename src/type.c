#include "type.h"
#include "varprivate.h"
#include "varutil.h"

// constructor

/*
 * for array and list varadic arguments should end with `NULL`
 * for array, list, and dict, arg type should be `var_t*`
 * for dict, arguments should be `var_t* key_arr, var_t* val_arr`
 * for string, format string is allowed
 * for rest of the types, it should be their corresponding C type. 
 *
 * @param   t   type of the `var_t`, can be char or enum 
 * @param   ... see above description
 * @return      pointer to a new `var_t`
 */
var_t* var_new(var_type_t t, ...) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);

    // start varadic arguments
    va_list ap;
    va_start(ap, t);
    
    switch (t) {
        case VAR_NIL: break;

        case VAR_INT: {
            res->data.i = va_arg(ap, int64_t);
        }
        break;

        case VAR_UINT: {
            res->data.u = va_arg(ap, uint64_t);
        }
        break;

        case VAR_FLOAT: {
            res->data.f = va_arg(ap, double);
        }
        break;

        case VAR_STRING: {
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

        case VAR_ARRAY: {
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

        case VAR_LIST: {
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

        case VAR_DICT: {
            var_t* key = va_arg(ap, var_t*);
            var_t* val = va_arg(ap, var_t*);
            res = var_new_dict(key, val);
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


var_t* var_news(const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    var_t* res = var_vnews(&format, ap);

    va_end(ap);
    return res;
}


var_t* var_vnews(const char** format, va_list ap) {
    var_t* res;

    switch (**format) {
        // var_t, any `var_t*` will be acceptable
        case 'v': {
            res = va_arg(ap, var_t*);
        }
        break;

        // nil
        case 'n': {
            res = malloc(sizeof (var_t));
            MEM_CHECK(res);
            res->type = VAR_NIL;
        }
        break;

        // int
        case 'i': {
            res = malloc(sizeof (var_t));
            MEM_CHECK(res);
            res->type = VAR_INT;
            res->data.i = va_arg(ap, int64_t);
        }
        break;

        // uint
        case 'u': {
            res = malloc(sizeof (var_t));
            MEM_CHECK(res);
            res->type = VAR_UINT;
            res->data.u = va_arg(ap, uint64_t); 
        }
        break;

        // float 
        case 'f': {
            res = malloc(sizeof (var_t));
            MEM_CHECK(res);
            res->type = VAR_FLOAT;
            res->data.f = va_arg(ap, double);
        }
        break;

        // string
        case 's': {
            res = malloc(sizeof (var_t));
            MEM_CHECK(res);
            res->type = VAR_STRING;
            char* str = va_arg(ap, char*);
            size_t str_len = strlen(str);
            res->data.s = malloc(sizeof (var_string_t) + sizeof (char) * (str_len + 1));
            MEM_CHECK(res->data.s);
            res->data.s->len = str_len;
            strcpy(res->data.s->str, str);
        }
        break;

        // array 
        case 'a': {
            res = va_arg(ap, var_t*);
            if (res->type != VAR_ARRAY) {
                ERRO("parsing failed");
            }
        }
        break;

        // array with initialization 
        case '(': {
            res = malloc(sizeof (var_t));
            MEM_CHECK(res);
            res->type = VAR_ARRAY;

            // get array length 
            size_t          a       = 1;            // nested array
            size_t          l       = 0;            // nested list
            size_t          len     = 0;            // array length 
            const char*     curr    = *format + 1;  // curr pos in format string 

            for (; a != 0; curr++) {
                switch (*curr) {
                    case '(': {
                        a++;
                    }
                    break;
                    case ')': {
                        a--;
                    }
                    break;
                    case '[': {
                        l++;
                    }
                    break;
                    case ']': {
                        l--;
                    }
                    break;
                }
                if (a == 1 && l == 0) {
                    len++;
                }
            }

            // allocate memory 
            res->data.a = malloc(sizeof (var_array_t) + sizeof (var_t*) * len);
            MEM_CHECK(res->data.a);

            // assign value
            (*format)++;
            res->data.a->len = len;
            for (size_t i = 0; i < len; i++) {
                res->data.a->av[i] = var_vnews(format, ap);
            }
        }
        break;

        // list 
        case 'l': {
            res = va_arg(ap, var_t*);
            if (res->type != VAR_LIST) {
                ERRO("parsing failed");
            }
        }
        break;

        // list with initalization
        case '[': {
            res = malloc(sizeof (var_t));
            MEM_CHECK(res);
            res->type = VAR_LIST;
            
            // get array length 
            size_t          a   = 0;            // nested array
            size_t          l   = 1;            // nested list
            size_t          len = 0;            // array length 
            const char*     c   = *format + 1;  // curr pos in format string 

            for (; l != 0; c++) {
                switch (*c) {
                    case '(': {
                        a++;
                    }
                    break;
                    case ')': {
                        a--;
                    }
                    break;
                    case '[': {
                        l++;
                    }
                    break;
                    case ']': {
                        l--;
                    }
                    break;
                }
                if (a == 0 && l == 1) {
                    len++;
                }
            }

            // allocate memory 
            res->data.l         = malloc(sizeof (var_list_t));
            res->data.l->len    = len;
            
            // total node count 
            size_t node_count   = (len + LIST_SIZE - 1) / LIST_SIZE;
            
            // allocate node memory 
            var_node_t* curr = &(res->data.l->lv);
            for (size_t i = 1; i < node_count; i++) {
                curr->next = malloc(sizeof (var_node_t));
                MEM_CHECK(curr->next);
                curr = curr->next;
            }
            curr->next = NULL;


            // assign values
            (*format)++;
            curr = &(res->data.l->lv);
            for (size_t i = 0; i < len; i += LIST_SIZE) {
                for (size_t j = 0; j < LIST_SIZE && i + j < len; j++) {
                    curr->vars[j] = var_vnews(format, ap);
                }
                curr = curr->next;
            }
        }
        break;

        // dict 
        case 'd': {
            res = va_arg(ap, var_t*);
            if (res->type != VAR_DICT) {
                ERRO("parsing failed");
            }
        }
        break;

        default: {
            ERRO("parsing failed");
        }
    }

    (*format)++;

    return res;
}


var_t* var_new_nil(void) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_NIL;
    return res;
}


var_t* var_new_int(int64_t i) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_INT;
    res->data.i = i;
    return res;
}


var_t* var_new_uint(uint64_t u) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_UINT;
    res->data.u = u;
    return res;
}


var_t* var_new_float(double f) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_FLOAT;
    res->data.f = f;
    return res;
}


/*
 * @param   s   format string 
 * @param   ... see `printf`
 * @return      a sized string 
 */
var_t* var_new_string(const char* s, ...) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_STRING;

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


var_t* var_new_array(var_t* var, ...) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_ARRAY;

    // return if len is 0
    if (var == NULL) {
        res->data.a = malloc(sizeof (var_array_t));
        MEM_CHECK(res->data.a);
        res->data.a->len = 0;
        return res;
    }

    va_list ap;
    va_start(ap, var);

    // get argument length
    size_t len = 1;
    for (var_t* temp = va_arg(ap, var_t*); 
        temp != NULL; 
        temp = (len++, va_arg(ap, var_t*)));
    va_end(ap);    
    va_start(ap, var);

    // alocate memory
    res->data.a = malloc(sizeof (var_array_t) + sizeof (var_t*) * len);
    MEM_CHECK(res->data.a);
    res->data.a->len = len;

    // assign values 
    res->data.a->av[0] = var;
    for (size_t i = 1; i < len; i++) {
        res->data.a->av[i] = va_arg(ap, var_t*);
    }

    va_end(ap);

    return res;
}


/*
 * @param   size    size of the array/struct/tuple 
 * @return          an empty array/struct/tuple with size `size`
 */
var_t* var_new_array_size(size_t size) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_ARRAY;
    res->data.a = malloc(sizeof (var_array_t) + sizeof (var_t*) * size);
    MEM_CHECK(res->data.a);
    res->data.a->len = size;
    return res;
}


/*
 * @param   var first element of the list, `NULL` if empty list
 * @param   ... the rest of the elements, should end with `NULL`
 * @return      a new `var_t*` of type `VAR_LIST`
 */
var_t* var_new_list(var_t* var, ...) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_LIST;

    // allocate struct memory 
    res->data.l = malloc(sizeof (var_list_t));
    MEM_CHECK(res->data.l);
    
    // return if len is 0
    if (var == NULL) {
        res->data.l->len = 0;
        res->data.l->lv.next = NULL;
        return res;
    }

    va_list ap;
    va_start(ap, var);

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
    for (size_t i = LIST_SIZE; i < res->data.l->len; i += LIST_SIZE) {
        for (size_t j = 0; j < LIST_SIZE && i + j < res->data.l->len; j++) {
            curr->vars[j] = va_arg(ap, var_t*);
        }
        curr = curr->next;
    }

    va_end(ap);

    return res;
}


/*
 * the length of key_arr should be the same with val_arr
 *
 * NOTE!
 * since list and dict are not hashable, they cannot be dict key
 * array containing them will also be unhashable, thus cannot be dict key
 * 
 * @param   key_arr array of keys
 * @param   val_arr array of vals
 * @return          a new `var_t*` of type `VAR_DICT`
 */
var_t* var_new_dict(var_t* key_arr, var_t* val_arr) {
    var_t* res = malloc(sizeof (var_t));
    MEM_CHECK(res);
    res->type = VAR_DICT;

    res->data.d = malloc(sizeof (var_dict_t));
    MEM_CHECK(res->data.d);
    res->data.d->mod = DICT_SIZE;
    
    // len = 0 if NULL 
    // get len
    size_t len;
    if (key_arr == NULL) {
        len = 0;
    } else {
        size_t len = key_arr->data.a->len;
        if (len != val_arr->data.a->len) {
            ERRO("key size must be exactly equal to val size");
        }
    }

    // alloate memory 
    res->data.d->list = malloc(sizeof (var_dict_list_t) * DICT_SIZE);
    MEM_CHECK(res->data.d->list);
    memset(res->data.d->list, 0, sizeof (var_dict_list_t) * DICT_SIZE);

    // assign values 
    size_t index;
    uint64_t hash;
    var_dict_elem_t* elem;
    for (size_t i = 0; i < len; i++) {
        elem = malloc(sizeof (var_dict_elem_t));
        MEM_CHECK(elem);
        elem->key = key_arr->data.a->av[i];
        elem->val = val_arr->data.a->av[i];
        if (var_hash(elem->key, &hash) == false) {
            ERRO("failed to hash")
        }
        elem->hash = hash;
        index = hash % DICT_SIZE;
        elem->prev = res->data.d->list[index].tail;
        elem->next = NULL;
        if (res->data.d->list[index].size++ == 0) {
            res->data.d->list[index].head = elem;
            res->data.d->list[index].tail = elem;
        } else {
            res->data.d->list[index].tail->next = elem;
            res->data.d->list[index].tail = elem;
        }
    }

    size_t max = 0;
    for (size_t i = 0; i < DICT_SIZE; i++) {
        if (res->data.d->list[i].size > max) {
            max = res->data.d->list[i].size;
        }
    }

    if (max <= DICT_SIZE) return res;
    max /= DICT_SIZE;

    var_dict_reshape(res->data.d, max);

    return res;
}


/*
 * free the memory used by `var_t*`
 *
 * @param   var the var you want to free 
 */
void var_delete(var_t* var) {
    (void) var;
    switch (var->type) {
        case VAR_INT: {
            free(var);
        }
        break;

        case VAR_UINT: {
            free(var);
        }
        break;

        case VAR_FLOAT: {
            free(var);
        }
        break;

        case VAR_STRING: {
            free(var->data.s);
            free(var);
        }
        break;

        case VAR_ARRAY: {
            var_array_t* arr = var->data.a;
            for (size_t i = 0; i < arr->len; i++) {
                var_delete(arr->av[i]);
            }
            free(var->data.a);
            free(var);
        }
        break;

        case VAR_LIST: {
            var_node_t* curr = &var->data.l->lv;
            var_node_t* next;
            for (size_t i = 0; i < LIST_SIZE && i < var->data.l->len; i++) {
                var_delete(curr->vars[i]);
            }
            for (size_t i = LIST_SIZE; i < var->data.l->len; i += LIST_SIZE) {
                for (size_t j = 0; j < LIST_SIZE && i + j < var->data.l->len; j++) {
                    var_delete(curr->vars[i]);
                }
                next = curr->next;
                free(curr);
                curr = next;
            }
            free(var->data.l);
            free(var);
        }
        break;

        case VAR_DICT: {
            var_dict_t* dict = var->data.d;
            var_dict_elem_t* curr;
            var_dict_elem_t* next;
            for (size_t i = 0; i < dict->mod; i++) {
                curr = dict->list[i].head;
                for (size_t j = 0; j < dict->list[i].size; j++) {
                    var_delete(curr->key);
                    var_delete(curr->val);
                    next = curr->next;
                    free(curr);
                    curr = next;
                }
            }
            free(dict->list);
            free(dict);
            free(var);
        }
        break;

        default: {
            ERRO("corrupted var");
        }
    }
}


/*
 * list and dict will not be hashable. 
 * int, uint, float, string, and array are hashable types. 
 *
 * @param   var     the `var_t*` that you want to get the hash code from
 * @param   hash    a pointer to a `uint64_t` that will be used to store the result
 * @return          if the `var_t*` is hashable or not, if hash succeeded
 */
bool var_hash(const var_t* var, uint64_t* hash) {
    switch (var->type) {
        case VAR_INT: {
            memcpy(hash, &var->data.i, sizeof (int64_t));
            return true;
        }

        case VAR_UINT: {
            memcpy(hash, &var->data.u, sizeof (uint64_t));
            return true;
        }

        case VAR_FLOAT: {
            memcpy(hash, &var->data.f, sizeof (double));
            return true;
        }

        case VAR_STRING: {
            // FNV-1a algorithm for string hashing
            *hash = (uint64_t) DICT_HASH;
            const unsigned char* ptr = (const unsigned char*) var->data.s->str;

            for (size_t i = 0; i < var->data.s->len; i++) {
                *hash ^= (uint64_t) (ptr[i]);
                *hash *= DICT_PRIME;
            }

            return true;
        }

        case VAR_ARRAY: {
            // combining hashes of all vars inside
            // old_hash ^= new_hash + 0x9e3779b97f4a7c15 + (old_hash << 6) + (old_hash >> 2);
            *hash = 0;
            uint64_t new_hash;
            for (size_t i = 0; i < var->data.a->len; i++) {
                if (var_hash(var->data.a->av[i], &new_hash) == false) {
                    return false;
                }
                *hash ^= new_hash + DICT_RATIO + (*hash << 6) + (*hash >> 2);
            }
            return true;
        }

        case VAR_LIST: {
            return false;
        }

        case VAR_DICT: {
            return false;
        }

        default: {
            ERRO("corrupted var");
        }
    }
    return false;
}


/*
 * `VAR_DICT` cannot be parsed by `var_get`
 *
 * @param   var     the `var_t*` that you want to get from
 * @param   format  format string of how you want to get the values 
 * @param   ...     pointer of variable
 */
void var_get(const var_t* var, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    var_vget(var, &format, ap);

    va_end(ap);
}


void var_vget(const var_t* var, const char** format, va_list ap) {
    while (**format == ' ') (*format)++;
    const char* ptr = *format;
    switch (var->type) {
        case VAR_NIL: {
            switch(*ptr) {
                case 'n':
                case 'v': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                case '_': break;

                default: {
                    ERRO("parsing failed, try to dereference nil");
                }
            }
        }
        break;

        case VAR_INT: {
            switch (*ptr) {
                case 'i': {
                    memcpy(va_arg(ap, int64_t*), &var->data.i, sizeof (int64_t));
                }
                break;
                case 'v': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_INT`");
                }
            }
        }
        break;

        case VAR_UINT: {
            switch (*ptr) {
                case 'u': {
                    memcpy(va_arg(ap, uint64_t*), &var->data.u, sizeof (uint64_t));
                }
                break;
                case 'v': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_UINT`");
                }
            }
        }
        break;

        case VAR_FLOAT: {
            switch (*ptr) {
                case 'f': {
                    memcpy(va_arg(ap, double*), &var->data.f, sizeof (double));
                }
                break;
                case 'v': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_FLOAT`");
                }
            }
        }
        break;

        case VAR_STRING: {
            switch (*ptr) {
                case 's': {
                    char** str_ptr = va_arg(ap, char**);
                    *str_ptr = var->data.s->str;
                }
                break;
                case 'v': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_STRING`");
                }
            }
        }
        break;

        case VAR_ARRAY: {
            switch (*ptr) {
                case 'a': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case 'v': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case '_': break;

                case '(': {
                    for (size_t i = (ptr++, 0); 
                        i < var->data.a->len && *ptr != '\0' && *ptr != ')'; 
                        i++) {
                        var_vget(var->data.a->av[i], &ptr, ap);
                    }
                }
                break;

                default: {
                    ERRO("parsing failed, expected type `VAR_ARRAY`");
                }
            }
        }
        break;

        case VAR_LIST: {
            switch (*ptr) {
                case 'l': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case 'v': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case '_': break;

                case '[': {
                    var_list_t* list = var->data.l;
                    var_node_t* curr = &list->lv;
                    for (size_t i = (ptr++, 0);
                        i < list->len && *ptr != '\0' && *ptr != ']';
                        i++) {
                        var_vget(curr->vars[i % LIST_SIZE], &ptr, ap);
                        if (i % LIST_SIZE == 0 && i != 0) {
                            curr = curr->next;
                        }
                    }
                }
                break;

                default: {
                    ERRO("parsing failed, expected type `VAR_LIST`");
                }
            }
        }
        break;

        case VAR_DICT: {
            switch (*ptr) {
                case 'd': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case 'v': {
                    memcpy(va_arg(ap, var_t**), &var, sizeof (var_t*));
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_DICT`");
                }
            }
        }
        break;

        default: {
            ERRO("corrupted type");
        }
        break;
    }

    (*format)++;
}


/*
 * data type:
 *      n: VAR_NIL  
 *      i: VAR_INT       
 *      u: VAR_UINT 
 *      f: VAR_FLOAT 
 *      l: VAR_LIST 
 *      d: VAR_DICT
 * 
 * other valid input: 
 *      _: ignored
 *      v: `var_t*`, it must be of the same type as before, it is passed by value, u need to destroy the original `var_t*` manually
 *      (): setting fields of array 
 *      []: setting fields of list 
 *
 *
 * @param   var     the `var_t*` that you want to change
 * @param   format  the format of the `var_t*`, see above
 * @param   ...     the argument of format
 */
void var_set(var_t* var, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    var_vset(var, &format, ap);

    va_end(ap);
}


void var_vset(var_t* var, const char** format, va_list ap) {
    while (**format == ' ') (*format)++;
    const char* ptr = *format;
    switch (var->type) {
        case VAR_NIL: {
            switch (*ptr) {
                case 'v': {
                    var_t* temp = va_arg(ap, var_t*);
                    *var = *temp;
                    // TODO: require `var_copy`
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `var_t*`");
                }
            }
        }
        break;

        case VAR_INT: {
            switch (*ptr) {
                case 'i': {
                    var->data.i = va_arg(ap, int64_t);
                }
                break;
                case 'n': {
                    var->type = VAR_NIL;
                }
                break;
                case 'v': {
                    var_t* temp = va_arg(ap, var_t*);
                    if (temp->type != VAR_INT) {
                        ERRO("parsing failed, expected type `VAR_INT`");
                    }
                    var->data = temp->data;
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_INT`");
                }
            }
        }
        break;

        case VAR_UINT: {
            switch (*ptr) {
                case 'u': {
                    var->data.u = va_arg(ap, uint64_t);
                }
                break;
                case 'n': {
                    var->type = VAR_NIL;
                }
                break;
                case 'v': {
                    var_t* temp = va_arg(ap, var_t*);
                    if (temp->type != VAR_UINT) {
                        ERRO("parsing failed, expected type `VAR_UINT`");
                    }
                    var->data = temp->data;
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_UINT`");
                }
            }
        }
        break;

        case VAR_FLOAT: {
            switch (*ptr) {
                case 'f': {
                    var->data.f = va_arg(ap, double);
                }
                break;
                case 'n': {
                    var->type = VAR_NIL;
                }
                break;
                case 'v': {
                    var_t* temp = va_arg(ap, var_t*);
                    if (temp->type != VAR_FLOAT) {
                        ERRO("parsing failed, expected type `VAR_FLOAT`");
                    }
                    var->data = temp->data;

                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_FLOAT`");
                }
            }
        }
        break;

        case VAR_STRING: {
            switch (*ptr) {
                case 's': {
                    char* str = va_arg(ap, char*);
                    size_t str_len = strlen(str);
                    var->data.s = realloc(var->data.s, sizeof (var_string_t) + sizeof (char) * (str_len + 1));
                    var->data.s->len = str_len;
                    strcpy(var->data.s->str, str);
                }
                break;
                case 'n': {
                    var->type = VAR_NIL;
                    free(var->data.s);
                }
                break;
                case 'v': {
                    var_t* temp = va_arg(ap, var_t*);
                    if (temp->type != VAR_STRING) {
                        ERRO("parsing failed, expected type `VAR_STRING`");
                    }
                    // TODO: require `var_copy`
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_STRING`");
                }
            }
        }
        break;

        case VAR_ARRAY: {
            switch (*ptr) {
                case 'a': {
                    *var = *va_arg(ap, var_t*);
                }
                break;
                case 'n': {
                    var->type = VAR_NIL;
                    free(var->data.a);
                }
                break;
                case 'v': {
                    *var = *va_arg(ap, var_t*);
                }
                break;
                case '_': break;

                case '(': {
                    for (size_t i = (ptr++, 0); 
                        i < var->data.a->len && *ptr != '\0' && *ptr != ')'; 
                        i++) {
                        var_vset(var->data.a->av[i], &ptr, ap);
                    }
                }
                break;

                default: {
                    ERRO("parsing failed, expected type `VAR_ARRAY`");
                }
            }
        }
        break;

        case VAR_LIST: {
            switch (*ptr) {
                case 'l': {
                    *var = *va_arg(ap, var_t*);
                }
                break;
                case 'n': {
                    var->type = VAR_NIL;
                    var_node_t* curr = &var->data.l->lv;
                    var_node_t* next;
                    for (size_t i = 0; i < LIST_SIZE && i < var->data.l->len; i++) {
                        var_delete(curr->vars[i]);
                    }
                    for (size_t i = LIST_SIZE; i < var->data.l->len; i += LIST_SIZE) {
                        for (size_t j = 0; j < LIST_SIZE && i + j < var->data.l->len; j++) {
                            var_delete(curr->vars[i]);
                        }
                        next = curr->next;
                        free(curr);
                        curr = next;
                    }
                    free(var->data.l);
                    free(var);
                }
                break;
                case 'v': {
                    *var = *va_arg(ap, var_t*);
                }
                break;
                case '_': break;

                case '[': {
                    var_list_t* list = var->data.l;
                    var_node_t* curr = &list->lv;
                    for (size_t i = (ptr++, 0);
                        i < list->len && *ptr != '\0' && *ptr != ']';
                        i++) {
                        var_vset(curr->vars[i % LIST_SIZE], &ptr, ap);
                        if (i % LIST_SIZE == 0 && i != 0) {
                            curr = curr->next;
                        }
                    }
                }
                break;

                default: {
                    ERRO("parsing failed, expected type `VAR_LIST`");
                }
            }
        }
        break;

        case VAR_DICT: {
            switch (*ptr) {
                case 'd': {
                    *var = *va_arg(ap, var_t*);
                }
                break;
                case 'n': {
                    var->type = VAR_NIL;
                    var_dict_t* dict = var->data.d;
                    var_dict_elem_t* curr;
                    var_dict_elem_t* next;
                    for (size_t i = 0; i < dict->mod; i++) {
                        curr = dict->list[i].head;
                        for (size_t j = 0; j < dict->list[i].size; j++) {
                            var_delete(curr->key);
                            var_delete(curr->val);
                            next = curr->next;
                            free(curr);
                            curr = next;
                        }
                    }
                    free(dict->list);
                    free(dict);
                    free(var);
                }
                break;
                case 'v': {
                    *var = *va_arg(ap, var_t*);
                }
                break;
                case '_': break;

                default: {
                    ERRO("parsing failed, expected type `VAR_DICT`");
                }
            }
        }
        break;

        default: {
            ERRO("corrupted type");
        }
        break;
    }

    (*format)++;
}

size_t var_len(const var_t* var) {
    switch (var->type) {
        case VAR_NIL: {
            ERRO("type `VAR_NIL` does not have len");
        }
        break;

        case VAR_INT: {
            ERRO("type `VAR_INT` does not have len");
        }
        break;

        case VAR_UINT: {
            ERRO("type `VAR_UINT` does not have len");
        }
        break;

        case VAR_FLOAT: {
            ERRO("type `VAR_FLOAT` does not have len");
        }
        break;

        case VAR_STRING: {
            return var->data.s->len;
        }
        break;

        case VAR_ARRAY: {
            return var->data.a->len;
        }
        break;

        case VAR_LIST: {
            return var->data.l->len;
        }
        break;

        case VAR_DICT: {
            size_t len = 0;
            for (size_t i = 0; i < var->data.d->mod; i++) {
                len += var->data.d->list->size;
            }
            return len;
        }
        break;

        default: {
            ERRO("unknown type");
        }
    }
    ERRO("unknown type");
}



