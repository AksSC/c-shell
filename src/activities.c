#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<signal.h>
#include "activities.h"
#include "command.h"
#include "prompt.h"

ProcessInfo processes[LEN];
int process_count = 0;

void add_process(const char *command, pid_t pid){
    processes[process_count].status = RUNNING;
    strcpy(processes[process_count].command, command);
    processes[process_count].pid = pid;
    process_count++;
}

void remove_terminated_process(pid_t pid){
    for(int i = 0; i < process_count; i++){
        if(processes[i].pid == pid){ // Check if process is terminated or not???
            for(int j = i; j < process_count - 1; j++){
                processes[j] = processes[j + 1];
            }
            process_count--;
            break;
        }
    }
}

char* update_status(pid_t pid, ProcessStatus status){
    for(int i = 0; i < process_count; i++){
        if(processes[i].pid == pid){
            processes[i].status = status;
            return processes[i].command;
        }
    }
    return NULL;
}

int compare_by_pid(const void *a, const void *b){
    pid_t pid_a = ((ProcessInfo *)a)->pid;
    pid_t pid_b = ((ProcessInfo *)b)->pid;
    return pid_a - pid_b;
}

void print_activities(){
    qsort(processes, process_count, sizeof(ProcessInfo), compare_by_pid);

    for(int i = 0; i < process_count; i++){
        if(processes[i].status != TERMINATED && processes[i].status != ABNORMAL){
            printf("%d : %s - %s\n", processes[i].pid, processes[i].command, processes[i].status == RUNNING ? "Running" : "Stopped");
        }
    }
}

void sigchld_handler(int signum){
    pid_t pid;
    int status;

    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0){
        if(WIFEXITED(status) || WIFSIGNALED(status)){
            char *cmd = update_status(pid, WIFSIGNALED(status) ? ABNORMAL : TERMINATED);
            remove_terminated_process(pid);
            if(cmd != NULL){
                printf("\n%s exited %s (%d)\n", cmd, WIFSIGNALED(status) ? "abnormally" : "normally", pid);
                prompt(0, "sleep");
                fflush(stdout);
            }
        }else if(WIFSTOPPED(status)){
            update_status(pid, STOPPED);
        }else if(WIFCONTINUED(status)){
            update_status(pid, RUNNING);
        }
    }
}

ProcessInfo* find_process(pid_t pid){
    for(int i = 0; i < process_count; i++){
        if(processes[i].pid == pid){
            return &processes[i];
        }
    }
    return NULL;
}

void stop_foreground_process(pid_t pid){
    ProcessInfo* proc = find_process(pid);
    if(proc != NULL){
        proc->status = STOPPED;
        printf("\n[%d] %s - Stopped\n", pid, proc->command);
    }
}

void continue_background_process(pid_t pid){
    ProcessInfo* proc = find_process(pid);
    if(proc != NULL){
        proc->status = RUNNING;
        printf("[%d] %s - Running\n", pid, proc->command);
    }
}
