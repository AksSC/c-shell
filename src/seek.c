#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include "seek.h"
#include<unistd.h>
#include "hop.h"
#include "command.h"

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE  "\033[34m"

extern char home[LEN];
extern char last_dir[LEN];

void seek_directory(const char *target, char *base_path, int flag_d, int flag_f, int flag_e, int *found, char *single_match){
    struct dirent *entry;
    struct stat statbuf;
    char path[LEN];
    char cwd[LEN];
    getcwd(cwd, sizeof(cwd));
    if(base_path[0] == '-'){
        chdir(last_dir);
        base_path[0] = '.';
    }
    if(base_path[0] == '~'){
        chdir(home);
        base_path[0] = '.';
    }
    DIR *dir = opendir(base_path);
    if(dir == NULL){
        return;
    }

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        if(lstat(path, &statbuf) == -1){
            continue;
        }

        if((flag_d && S_ISDIR(statbuf.st_mode)) || (flag_f && S_ISREG(statbuf.st_mode)) || (!flag_d && !flag_f)){
            if(strncmp(entry->d_name, target, strlen(target)) == 0){
                (*found)++;
                if(*found == 1){
                    strcpy(single_match, path);
                }
                if(S_ISDIR(statbuf.st_mode)){
                    printf(COLOR_BLUE "%s\n" COLOR_RESET, path);
                }else{
                    printf(COLOR_GREEN "%s\n" COLOR_RESET, path);
                }
            }
        }

        if(S_ISDIR(statbuf.st_mode)){
            seek_directory(target, path, flag_d, flag_f, flag_e, found, single_match);
        }
    }

    closedir(dir);
    chdir(cwd);
}

void seek_cmd(char *cmd){
    int flag_d = 0, flag_f = 0, flag_e = 0;
    char *target = NULL;
    char *directory = ".";

    char *token = strtok(cmd, " ");
    token = strtok(NULL, " ");
    while (token != NULL){
        if(strcmp(token, "-d") == 0){
            flag_d = 1;
        }else if(strcmp(token, "-f") == 0){
            flag_f = 1;
        }else if(strcmp(token, "-e") == 0){
            flag_e = 1;
        }else if(!target){
            target = token;
        }else{
            directory = token;
        }
        token = strtok(NULL, " ");
    }

    if(!target){
        printf("ERROR: No target specified!\n");
        return;
    }

    if(flag_d && flag_f){
        printf("Invalid flags!\n");
        return;
    }

    int found = 0;
    char single_match[LEN] = "";

    seek_directory(target, directory, flag_d, flag_f, flag_e, &found, single_match);

    if(found == 0){
        printf("No match found!\n");
    }else if(found == 1 && flag_e){
        struct stat statbuf;
        if(stat(single_match, &statbuf) == 0){
            if(S_ISDIR(statbuf.st_mode)){
                if(access(single_match, X_OK) == 0){
                    chdir(single_match);
                    printf(COLOR_BLUE "%s\n" COLOR_RESET, single_match);
                }else{
                    printf("Missing permissions for task!\n");
                }
            }else if(S_ISREG(statbuf.st_mode)){
                if(access(single_match, R_OK) == 0){
                    FILE *file = fopen(single_match, "r");
                    if(file){
                        char line[LEN];
                        while(fgets(line, sizeof(line), file)){
                            printf("%s", line);
                        }
                        fclose(file);
                    }
                }else{
                    printf("Missing permissions for task!\n");
                }
            }
        }
    }
}
