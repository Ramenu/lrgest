#include <stdlib.h>
#include <errno.h>
#include "vec.h"
#include <string.h>

vec *vec_init(void)
{
    vec *v = malloc(sizeof(vec));
    
    if (v == NULL)
        return NULL;

    v->data = malloc(sizeof(dir_info));
    v->size = 0;
    return v;
}

int vec_push(vec *self, const dir_info *elem)
{
    if (self == NULL || elem == NULL)
        return VEC_NULL_DATA;
    
    self->data = realloc(self->data, sizeof(dir_info) * (self->size + 1));

    if (self->data == NULL)
        return ENOMEM;

    ++self->size;
    self->data[self->size - 1] = *elem;
    return VEC_SUCCESS;
}

int vec_pop_back(vec *self)
{
    if (self == NULL)
        return VEC_NULL_DATA;
    
    /* Make sure that the list won't be null */
    if (self->size <= 1)
        return VEC_FAILURE;

    self->data = realloc(self->data, sizeof(dir_info) * (self->size - 1));

    if (self->data == NULL)
        return VEC_FAILURE;

    --self->size;
    return VEC_SUCCESS;
}

int vec_remove(vec *self, usize index)
{
    if (self == NULL)
        return VEC_NULL_DATA;
    if (index >= self->size)
        return VEC_OUT_OF_BOUNDS;
    
    memmove(self->data + index, 
            self->data + index + 1,
            sizeof(dir_info) * (self->size - 1 - index));

    self->data = realloc(self->data, sizeof(dir_info) * (self->size - 1));
    if (self->data == NULL)
        return VEC_FAILURE;

    --self->size;
    return VEC_SUCCESS;
}

int vec_free(vec *self)
{
    if (self == NULL)
        return VEC_NULL_DATA;
    if (self->data == NULL)
        return VEC_NULL_DATA;

    for (usize i = 0; i < self->size; ++i)
        if (self->data[i].path != NULL)
            free(self->data[i].path);

    free(self->data);
    free(self);
    return VEC_SUCCESS;
}
