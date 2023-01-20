#ifndef LRGEST_VEC_H
#define LRGEST_VEC_H

#include "dir_info.h"
#include "types.h"

#define VEC_SUCCESS 0
#define VEC_FAILURE -1 /* Should be used only when it is a vague error and no other error code exists */
#define VEC_OUT_OF_BOUNDS -2
#define VEC_NULL_ELEMENT -3
#define VEC_NO_ELEMENTS -4
#define VEC_NULL_DATA -5


typedef struct vec
{
    dir_info *data;
    usize size;
} vec;

extern vec *vec_init(void);
extern int vec_push(vec *self, const dir_info *elem);
extern int vec_pop_back(vec *self);
extern int vec_remove(vec *self, usize index);
extern int vec_free(vec *self);
static inline dir_info *vec_at(const vec *self, usize i)
{
    #ifndef NDEBUG
        if (self == NULL)
            return NULL;
        if (i >= self->size)
            return NULL;
    #endif
    return &self->data[i];
}


#endif /* LRGEST_VEC_H */
