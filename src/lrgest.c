#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "color.h"
#include "vec.h"

static inline void error(const char *msg);


int main(int argc, char **argv)
{
    if (argc < 2) {
        error("no directory specified");
        return EXIT_FAILURE;
    }

    vec *dirs = vec_init();
    dir_info info;

    for (int i = 1; i < argc; ++i) {
        info.path = argv[i];
        info.size = 3;
        vec_push(dirs, &info);
    }

    vec_remove(dirs, 1);
    vec_remove(dirs, 2);
    vec_remove(dirs, 10);

    if (dirs != NULL) {
        for (usize i = 0; i < dirs->size; ++i) {
            dir_info *d = vec_at(dirs, i);
            if (d == NULL) {
                printf("NULL ABORTING\n");
                return EXIT_FAILURE;
            }
            printf("%s\n", d->path);
        }
    }

    vec_free(dirs);

    return 0;
}

static inline void error(const char *msg) 
{
    fprintf(stderr, COLOR_BOLDRED "error" COLOR_BOLDWHITE ":" COLOR_RESET " %s\n", msg);
}
