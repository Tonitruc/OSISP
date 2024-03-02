#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <locale.h>

#define PARAMS_LIST "dfls"
#define RESET_OPTIND 1
#define CURRENT_DIRECTORY "."

typedef enum SEARCH_PARAMS {
    NONE_OPTION,
    IS_DIRECTORY,
    IS_FILE,
    IS_SYM_LINK,
    IS_SORT,
    IS_ANY_OPTIONS = IS_DIRECTORY | IS_FILE | IS_SYM_LINK | IS_SORT
} SEARCH_PARAMS;

int compare_by_alph(const void* first, const void* second) {
    return strcoll(first, second);
}

typedef struct array {
    char **buffer;
    size_t capacity;
    size_t cur_size;
}array;
void resize_array(array *arr) {
    arr->capacity *= 2;
    arr->buffer = (char**)realloc(arr->buffer, arr->capacity * sizeof(char*));
}
void add_array(array *arr, char* str) {
    if(arr->cur_size + 1 == arr->capacity)
        resize_array(arr);
    arr->buffer[arr->cur_size] = (char*)malloc(strlen(str) + 1);
    strncpy(arr->buffer[arr->cur_size++], str, strlen(str) + 1);
}
void free_array(array *arr) {
    for(int i = 0; i < arr->cur_size; i++)
        free(arr->buffer[i]);
    free(arr->buffer);
}

void find_path_param(int argc, char** argv) {
    while(getopt(argc, argv, PARAMS_LIST) != -1) {}
}
bool is_dir_exist(const char *path) {
    DIR *dir;

    if((dir = opendir(path)) == NULL)
        return false;

    closedir(dir);
    return true;
}

void dirwalk(array* result, const char* path, SEARCH_PARAMS options) {
    DIR *dir;
    struct stat st;
    struct dirent* d;

    dir = opendir(path);
    while((d = readdir(dir))) {
        char newPath[MAXPATHLEN];
        if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
            continue;
        sprintf(newPath, "%s/%s", path, d->d_name);

        if (lstat(newPath, &st) == -1)
            continue;
        if(S_ISDIR(st.st_mode)) {
            if(options & IS_DIRECTORY)
                add_array(result, newPath);
            dirwalk(result, newPath, options);
        }
        else if(S_ISREG(st.st_mode) && (options & IS_FILE))
            add_array(result, newPath);
        else if(S_ISLNK(st.st_mode) && (options & IS_SYM_LINK))
            add_array(result, newPath);
    }

    closedir(dir);
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    array result = {NULL, 1, 0};
    char root_dir[MAXPATHLEN] = CURRENT_DIRECTORY;
    find_path_param(argc, argv);
    if(optind < argc)
        strncpy(root_dir, argv[optind], strlen(argv[optind]));

    optind = RESET_OPTIND;
    if(!is_dir_exist(root_dir)) {
        printf("There is no such directory\n");
        strncpy(root_dir, CURRENT_DIRECTORY, strlen(CURRENT_DIRECTORY) + 1);
    }

    printf("%s", root_dir);

    int opt; SEARCH_PARAMS options = NONE_OPTION;
    while((opt = getopt(argc, argv, PARAMS_LIST)) != -1) {
        switch (opt) {
            case 'd': {
                options |= IS_DIRECTORY;
                break;
            }
            case 'f': {
                options |= IS_FILE;
                break;
            }
            case 'l': {
                options |= IS_SYM_LINK;
                break;
            }
            case 's': {
                options |= IS_SORT;
                break;
            }
            default: break;
        }
    }
    if(options == NONE_OPTION)
        options = IS_DIRECTORY | IS_FILE | IS_SYM_LINK;
    dirwalk(&result, root_dir, options);
    if(options & IS_SORT)
        qsort(result.buffer, result.cur_size, sizeof(char*), compare_by_alph);
    for(int i = 0; i < result.cur_size; i++) {
        printf("\n%s", result.buffer[i]);
    }

    free_array(&result);
    return 1;
}
