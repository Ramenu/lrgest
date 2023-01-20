#ifndef LRGEST_DIRINFO_H
#define LRGEST_DIRINFO_H

#include <sys/types.h>

typedef struct dir_info
{
    char *path;
    off_t size;
} dir_info;

#endif /* LRGEST_DIRINFO_H */
