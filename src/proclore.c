#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<limits.h>
#include<errno.h>
#include<fcntl.h>
#include "proclore.h"
#include "command.h"

extern char home[LEN];

void proclore(const char *pid_arg){
    char proc_path[LEN];
    int process_pid;

    // Determine the PID to use
    if(pid_arg == NULL){
        process_pid = getpid();
    }else{
        process_pid = atoi(pid_arg);
    }
    snprintf(proc_path, sizeof(proc_path), "/proc/%d", process_pid);

    char status_file[LEN], exe_file[LEN];
    snprintf(status_file, sizeof(status_file), "%s/status", proc_path);
    snprintf(exe_file, sizeof(exe_file), "%s/exe", proc_path);

    FILE *fp = fopen(status_file, "r");
    if(!fp){
        perror("ERROR: Could not open status file");
        return;
    }

    char line[256];
    int pgrp = -1, vsize = -1;
    char state[8] = "Unknown";

    while(fgets(line, sizeof(line), fp)){
        if(strncmp(line, "State:", 6) == 0){
            sscanf(line, "State:\t%s", state);
        }else if(strncmp(line, "Pid:", 4) == 0){
            sscanf(line, "Pid:\t%d", &process_pid);
        }else if(strncmp(line, "VmSize:", 7) == 0){
            sscanf(line, "VmSize:\t%d", &vsize);
        }else if(strncmp(line, "Tgid:", 5) == 0){
            sscanf(line, "Tgid:\t%d", &pgrp);
        }
    }
    fclose(fp);

    char exe_path[LEN];
    ssize_t len = readlink(exe_file, exe_path, sizeof(exe_path) - 1);
    if(len != -1){
        exe_path[len] = '\0';
    }else{
        strcpy(exe_path, "Unknown");
    }

    if(state[1] == '\0' && tcgetpgrp(STDIN_FILENO) == pgrp){
        strcat(state, "+");
    }
    // Relative path
    if(strncmp(exe_path, home, strlen(home)) == 0){
        strcpy(exe_path, exe_path + strlen(home) - 1);
        exe_path[0] = '~';
    }

    printf("PID : %d\n", process_pid);
    printf("Process Status : %s\n", state);
    printf("Process Group : %d\n", pgrp);
    printf("Virtual Memory : %d kB\n", vsize);
    printf("Executable Path : %s\n", exe_path);
}
