#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "color.h"
#include "vec.h"
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>

#define ERROR_MSG COLOR_BOLDRED "error" COLOR_BOLDWHITE ":" COLOR_RESET
#define FILE_ACCESS_ERROR -1
#define _DT_DIR 4
#define DNAME_MAX_SIZE 256
#define GET_BUF_SIZE_BYTES(current_size) (DNAME_MAX_SIZE + (current_size))
#define LENGTH_OF(arr) (sizeof((arr)) / sizeof((arr[0])))

static off_t dir_sum(char *path, vec *dirs, usize num_calls);
static inline void abort_enomem(void);
static inline bool len_over_one(const char *str);


int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, ERROR_MSG " no directory specified\n");
        return 1;
    }
    if (access(argv[1], F_OK) == FILE_ACCESS_ERROR) {
        fprintf(stderr, ERROR_MSG " directory '%s' does not exist\n", argv[1]);
        return 1;
    }

    const usize len = strlen(argv[1]);
    char *directory = malloc(sizeof(char) * (len + 2));

    if (directory == NULL)
        abort_enomem();

    strcpy(directory, argv[1]);
    if (len_over_one(directory))
        if (directory[0] != '/')
            strcat(directory, "/");

    vec *dirs = vec_init();

    if (dirs == NULL)
        abort_enomem();

    dir_sum(directory, dirs, GET_BUF_SIZE_BYTES(len + 1));

    for (usize i = 0; i < dirs->size; ++i)
        if (dirs->data[i].path != NULL)
            printf("%s: %ld\n", dirs->data[i].path, dirs->data[i].size);
    
    vec_free(dirs);
    free(directory);

    return 0;
}

static off_t dir_sum(char *path, 
                     vec *dirs, 
                     usize current_buf_size)
{
    #ifndef NDEBUG
        if (access(path, F_OK) == FILE_ACCESS_ERROR) {
            fprintf(stderr, ERROR_MSG " directory '%s' does not exist\n", path);
            exit(EXIT_FAILURE);
        }
    #endif
    struct stat buf;
    struct dirent *sdir;
    off_t size = 0;
    DIR *dptr = opendir(path);

    /* couldn't open dir so just return 0 */
    if (dptr == NULL)
        return 0;

    /* +2 for '/' and '\0' */
    const usize new_buf_size = (current_buf_size + DNAME_MAX_SIZE + 2) * sizeof(char);
    char *dir_path = malloc(new_buf_size);
    if (dir_path == NULL)
        abort_enomem();
    char *file_path = malloc(new_buf_size);

    if (file_path == NULL)
        abort_enomem();

    while ((sdir=readdir(dptr)) != NULL) {
        if (sdir->d_type == _DT_DIR) {
            if (len_over_one(sdir->d_name)) {
                if (strcmp(sdir->d_name, ".") != 0 && strcmp(sdir->d_name, "..") != 0) {
                    strcpy(dir_path, path);
                    strcat(dir_path, sdir->d_name);
                    strcat(dir_path, "/");
                    stat(sdir->d_name, &buf);
                    size += dir_sum(dir_path, dirs, new_buf_size);
                }
            }
        }
        else {
            strcpy(file_path, path);
            strcat(file_path, sdir->d_name);
            stat(file_path, &buf);
            size += buf.st_size;
        }
    }

    dir_info *info = malloc(sizeof(dir_info));

    if (info == NULL)
        abort_enomem();

    info->path = malloc(sizeof(char) * current_buf_size);
    if (info->path == NULL)
        abort_enomem();

    strcpy(info->path, path);
    info->size = size;
    if (vec_push(dirs, info) == ENOMEM)
        abort_enomem();

    closedir(dptr);
    free(info);
    free(dir_path);
    free(file_path);

    return size;

}

static inline void abort_enomem(void)
{
    fprintf(stderr, ERROR_MSG " out of memory\n");
    exit(EXIT_FAILURE);
}

static inline bool len_over_one(const char *str)
{
    usize i = 0;
    while (str[i] != '\0' && i < 2)
        ++i;
    return i > 1;
}
