#ifndef __VARUTIL_H__
#define __VARUTIL_H__

#include "type.h"
#include "varprivate.h"


// error msgs
#define ERRO(msg) \
exit((fprintf(stderr, "[ERRO]: " msg "\n"), 1));

// check if allocation returns `NULL`
#define MEM_CHECK(ptr) \
if (ptr == NULL) ERRO("out of memory");



// reshape dictionary
static inline void var_dict_reshape(var_dict_t* dict, size_t step) {
    size_t old_size = dict->mod;
    size_t new_size = old_size * step * 2;

    var_dict_list_t* old_list = dict->list;
    var_dict_list_t* new_list = malloc(sizeof (var_dict_list_t) * new_size);
    MEM_CHECK(new_list);

    dict->mod   = new_size;
    dict->list  = new_list;

    memset(dict->list, 0, sizeof (var_dict_list_t) * new_size);

    var_dict_elem_t* curr;
    var_dict_elem_t* next;
    uint64_t index;
    for (size_t i = 0; i < old_size; i++) {
        curr = old_list[i].head;
        while (curr != NULL) {
            next    = curr->next;
            index   = curr->hash % new_size;
            
            if ( new_list[index].head == NULL ) {
                new_list[index].head = new_list[index].tail = curr;
                curr->prev = NULL;
            } else {
                new_list[index].tail->next = curr;
                new_list[index].tail = curr;
                curr->prev = new_list[index].tail;
            }
            new_list[index].size++;
            curr = next;
        }
    }

    for ( size_t i = 0; i < new_size; i++ ) {
        new_list[i].tail->next = NULL;
    }

    free(old_list);
}


#endif  // __VARUTIL_H__
