#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<limits.h>
#include "hop.h"
#include "command.h"

extern char home[LEN];
char last_dir[LEN] = "";  // For - flag


void hop(char *path){
    char cwd[LEN];
    getcwd(cwd, sizeof(cwd));
    if(strcmp(path, "-") == 0){
        if(strlen(last_dir) == 0){
            printf("ERROR: No previous directory available\n");
            return;
        }
        if(chdir(last_dir) != 0){
            perror("ERROR");
            return;
        }
        strcpy(last_dir, cwd);
    }else{
        strcpy(last_dir, cwd);
        if(path[0] == '~'){
            if(chdir(home) != 0) perror("ERROR");
            path[0] = '.';
        }
        if(chdir(path) != 0){
            perror("ERROR");
        }
    }
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}

void hop_cmd(char *input){
    char *token = strtok(input, " ");
    token = strtok(NULL, " ");

    if(token == NULL){
        hop(home);
    }else{
        char *toks[64];
        int i = 0;
        toks[i] = token;
        while(toks[i] != NULL){
            toks[++i] = strtok(NULL, " ");
        }
        for(int j = 0; toks[j] != NULL; j++){
            hop(toks[j]);
        }
    }
}
