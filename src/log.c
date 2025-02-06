#include "log.h"
#include<string.h>
#include "command.h"
#include<unistd.h>
#include "alias.h"

extern char home[LEN];
static char log_history[15][LEN];
static int log_index = 0;
static char log_file_path[1100];
static char last_cmd[LEN];

void load_log(){
    strcpy(log_file_path, home);
    strcat(log_file_path, "/.log");

    FILE *file = fopen(log_file_path, "r");
    if(file == NULL) return;

    log_index = 0;
    while(fgets(log_history[log_index], sizeof(log_history[log_index]), file)){
        log_history[log_index][strcspn(log_history[log_index], "\n")] = 0;
        log_index++;
    }
    fclose(file);
}

void save_log(){
    strcpy(log_file_path, home);
    strcat(log_file_path, "/.log");
    FILE *file = fopen(log_file_path, "w");
    if(file == NULL){
        perror("ERROR");
        return;
    }

    for(int i = 0; i < log_index; i++) {
        fprintf(file, "%s\n", log_history[i]);
    }
    fclose(file);
}

void logadd(const char *command){
    load_log();
    if(log_index > 0 && strcmp(log_history[(log_index - 1)], command) == 0){
        return;
    }
    char cmd_copy[LEN];
    strcpy(cmd_copy, command);
    char *token = strtok(cmd_copy, ";");

    while(token != NULL){
        // Make sure trimm is like this everywhere in every file!
        token = trimm(token);
        
        char *aliased = check_alias(token);
        if(aliased != NULL){
            if(strcmp(aliased, "log") == 0 || 
                strncmp(aliased, "log ", 4) == 0){
                return; // Don't log
            }
        }else{
            if(strcmp(token, "log") == 0 || 
                strncmp(token, "log ", 4) == 0){
                return; // Don't log
            }
        }
        
        token = strtok(NULL, ";");
    }
    if(log_index == 15){
        strcpy(last_cmd, log_history[0]);
        for(int i = 1; i < 15; i++){
            strcpy(log_history[i-1], log_history[i]);
        }
        strcpy(log_history[14], command);
    }else{
        strcpy(log_history[log_index], command);
        log_index++;
    }

    save_log();
}

void display_log(){
    load_log();
    for(int i = 0; i < log_index; i++){
        printf("%s\n", log_history[i]);
    }
}

void purge_log(){
    log_index = 0;
    strcpy(log_file_path, home);
    strcat(log_file_path, "/.log");
    remove(log_file_path);
}

double execute_log_command(int index){
    if(index < 1 || index > log_index){
        printf("ERROR: Invalid log index\n");
        return 0;
    }

    char command[LEN];
    if(index == 15){
        strcpy(command, last_cmd);
    }else{
        strcpy(command, log_history[(log_index - index)]);
    }
    logadd(command);

    // Execute
    command[strcspn(command, "\n")] = 0;
    trimm(command);
    char tmp[LEN];
    strcpy(tmp, command);
    char *commandss[LEN];
    int bg[LEN] = {0};
    commandss[0] = strtok(command, ";");
    int i = 0;
    while(commandss[i] != NULL){
        commandss[++i] = strtok(NULL, ";");
    }
    char *commands[LEN];
    int ctr = 0;
    for(int j = 0; j < i; j++){
        commands[ctr] = commandss[j];
        int flag = 0;
        trimm(commandss[j]);
        if(commandss[j][strlen(commandss[j]) - 1] == '&'){
            flag = 1;
            commandss[j][strlen(commandss[j]) - 1] = '\0';
        }
        char *tmpp = strtok(commandss[j], "&");
        trimm(tmpp);
        while(tmpp != NULL){
            commands[ctr] = tmpp;
            ctr++;
            tmpp = strtok(NULL, "&");
            if(tmpp != NULL){
                bg[ctr - 1] = 1;
            }else{
                if(flag) bg[ctr - 1] = 1;
            }
        }
    }
    double elapsed = 0;
    for(int j = 0; j < ctr; j++){
        if(bg[j] == 0){
            elapsed += execute(commands[j], bg[j]);
        }else{
            execute(commands[j], bg[j]);
        }
    }
    return elapsed;
}
