#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include "alias.h"
#include "command.h"

// IS THIS FINE?

#define MAX_ALIASES 1000
#ifndef LEN
#define LEN 4096
#endif

typedef struct{
    char name[LEN];
    char command[LEN];
} Alias;

Alias aliases[MAX_ALIASES];
int count = 0;

void init_aliases(char *home){
    if(home == NULL){
        perror("Error getting home directory");
        return;
    }

    char aliaspath[LEN];
    snprintf(aliaspath, sizeof(aliaspath), "%s/.myshrc", home);

    FILE *file = fopen(aliaspath, "r");
    if(file == NULL){
        perror("Error opening alias file");
        return;
    }

    char line[LEN];
    while(fgets(line, sizeof(line), file)){
        char *comment = strchr(line, '#');
        if(comment){
            *comment = '\0';
        }
        char *name = strtok(line, "=");
        char *command = strtok(NULL, "\n");

        if(name && command){
            name = trimm(name);
            command = trimm(command);

            strcpy(aliases[count].name, name);
            strcpy(aliases[count].command, command);
            count++;
        }
    }
    fclose(file);
}

char* check_alias(char *command){
    for(int i = 0; i < count; i++){
        if(strncmp(command, aliases[i].name, strlen(aliases[i].name)) == 0){
            static char full_command[LEN];
            snprintf(full_command, sizeof(full_command), "%s%s", aliases[i].command, command + strlen(aliases[i].name));
            return full_command;
        }
    }
    return NULL;
}
