#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include<sys/types.h>

typedef enum {
    RUNNING,
    STOPPED,
    TERMINATED,
    ABNORMAL // Terminated abnormally
} ProcessStatus;

typedef struct ProcessInfo {
    char command[4096];
    pid_t pid;
    ProcessStatus status;
} ProcessInfo;

void add_process(const char *command, pid_t pid);
void remove_terminated_process(pid_t pid);
char* update_status(pid_t pid, ProcessStatus status);
void print_activities();
void sigchld_handler(int signum);
ProcessInfo* find_process(pid_t pid);
void continue_background_process(pid_t pid);
void stop_foreground_process(pid_t pid);

#endif