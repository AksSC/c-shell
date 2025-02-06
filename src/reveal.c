#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<time.h>
#include<limits.h>
#include<pwd.h>
#include<grp.h>
#include<unistd.h>
#include "reveal.h"
#include "command.h"

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE  "\033[34m"

extern char home[LEN];
extern char last_dir[LEN];

int hidden(const char *name){
    return name[0] == '.';
}

int is_executable(struct stat st){
    return st.st_mode & S_IXUSR;
}

void print_file_details(const char *name, const char *path, struct stat st){
    printf((S_ISDIR(st.st_mode)) ? "d" : "-");
    printf((st.st_mode & S_IRUSR) ? "r" : "-");
    printf((st.st_mode & S_IWUSR) ? "w" : "-");
    printf((st.st_mode & S_IXUSR) ? "x" : "-");
    printf((st.st_mode & S_IRGRP) ? "r" : "-");
    printf((st.st_mode & S_IWGRP) ? "w" : "-");
    printf((st.st_mode & S_IXGRP) ? "x" : "-");
    printf((st.st_mode & S_IROTH) ? "r" : "-");
    printf((st.st_mode & S_IWOTH) ? "w" : "-");
    printf((st.st_mode & S_IXOTH) ? "x" : "-");

    printf(" %ld", st.st_nlink);
    printf(" %s %s", getpwuid(st.st_uid)->pw_name, getgrgid(st.st_gid)->gr_name);
    printf(" %5ld", st.st_size);

    char time_buf[128];
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", localtime(&st.st_mtime));
    printf(" %s", time_buf);

    if(S_ISDIR(st.st_mode)){
        printf(COLOR_BLUE "%s\n" COLOR_RESET, name);
    }else if(is_executable(st)){
        printf(COLOR_GREEN "%s\n" COLOR_RESET, name);
    }else{
        printf(" %s\n", name);
    }
}

int compare_names(const void *a, const void *b){
    return strcmp(*(const char **)a, *(const char **)b);
}

void reveal_cmd(char *input){
    char *path = ".";
    int show_hidden = 0;
    int show_details = 0;

    char *token = strtok(input, " ");
    token = strtok(NULL, " ");
    while(token != NULL){
        if(token[0] == '-'){
            if(strchr(token, 'a')) show_hidden = 1;
            if(strchr(token, 'l')) show_details = 1;
            if(strlen(token) == 1){
                if(strlen(last_dir) == 0){
                    printf("ERROR: No previous directory available\n");
                    return;
                }
                path = last_dir;
            }
        }else{
            path = token;
        }
        token = strtok(NULL, " ");
    }
    char cwd[LEN];
    getcwd(cwd, sizeof(cwd));
    if(path[0] == '~'){
        chdir(home);
        path[0] = '.';
    }

    struct stat st;
    if (stat(path, &st) == -1) {
        perror("ERROR");
        return;
    }

    if(!S_ISDIR(st.st_mode)){
        print_file_details(path, path, st);
        return;
    }
    
    DIR *dir = opendir(path);
    if(dir == NULL){
        perror("ERROR");
        return;
    }

    struct dirent *entry;
    char filepath[LEN];

    char **filenames = NULL;
    size_t file_count = 0;

    while((entry = readdir(dir)) != NULL){
        if(!show_hidden && hidden(entry->d_name)) continue;

        filenames = realloc(filenames, sizeof(char *) * (file_count + 1));
        filenames[file_count] = strdup(entry->d_name);
        file_count++;
    }

    qsort(filenames, file_count, sizeof(char *), compare_names);

    for(size_t i = 0; i < file_count; i++){
        snprintf(filepath, sizeof(filepath), "%s/%s", path, filenames[i]);
        if(stat(filepath, &st) == -1){
            perror("ERROR");
            continue;
        }

        if(show_details){
            print_file_details(filenames[i], filepath, st);
        }else{
            if(S_ISDIR(st.st_mode)){
                printf(COLOR_BLUE "%s  " COLOR_RESET, filenames[i]);
            }else if(is_executable(st)) {
                printf(COLOR_GREEN "%s  " COLOR_RESET, filenames[i]);
            }else{
                printf("%s  ", filenames[i]);
            }
        }

        free(filenames[i]);
    }

    free(filenames);
    if(!show_details) printf("\n");
    closedir(dir);
    chdir(cwd);
}