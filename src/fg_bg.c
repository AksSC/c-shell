#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<time.h>
#include<fcntl.h>
#include "fg_bg.h"
#include "activities.h"
#include<string.h>
#include<fcntl.h>
#include<ctype.h>

extern volatile sig_atomic_t fg_pid;
extern volatile sig_atomic_t fg_running;
extern int process_count;

double fg(pid_t pid){
    ProcessInfo* process = find_process(pid);
    if(process == NULL){
        printf("No such process found\n");
        return 0;
    }

    if(process->status == STOPPED){
        kill(pid, SIGCONT);
    }

    update_status(pid, RUNNING);

    fg_pid = pid;
    fg_running = 1;

    tcsetpgrp(STDIN_FILENO, pid);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int status;    
    waitpid(pid, &status, WUNTRACED);

    tcsetpgrp(STDIN_FILENO, getpgrp());

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    fg_pid = 0;
    fg_running = 0;
    if(WIFSTOPPED(status)){
        update_status(pid, STOPPED);
        printf("\nProcess %d stopped\n", pid);
    }else if(WIFEXITED(status) || WIFSIGNALED(status)){
        remove_terminated_process(pid);
    }
    return elapsed;
}

void bg_func(pid_t pid){
    ProcessInfo* process = find_process(pid);
    if(process == NULL){
        printf("No such process found\n");
        return;
    }

    if(process->status != STOPPED){
        printf("Process is not stopped\n");
        return;
    }
    kill(pid, SIGCONT);

    update_status(pid, RUNNING);

    printf("[%d] %d\n", process_count, pid);
}