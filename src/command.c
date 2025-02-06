#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<ctype.h>
#include "hop.h"
#include<time.h>
#include "reveal.h"
#include "command.h"
#include "log.h"
#include "proclore.h"
#include "seek.h"
#include "iMan.h"
#include "neonate.h"
#include "activities.h"
#include<signal.h>
#include "fg_bg.h"
#include<fcntl.h>

extern int process_count;
volatile sig_atomic_t fg_pid = 0;
volatile sig_atomic_t fg_running = 0;

typedef struct{
    pid_t pid;
    char cmd[256];
} Proc;

// Remove extra whitespace (start and end)
char *trimm(char *str){
    while(isspace((unsigned char)*str)) str++;
    // printf("str: %s\n", str);
    if(*str == 0) return str;
    char *end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}


// Or write in readme that ^C shows up on prompt but can't remove it but does nothing either...
void ping_command(char *cmd){
    cmd = strtok(cmd, " ");
    char *pid_str = strtok(NULL, " ");
    char *signal_str = strtok(NULL, " ");

    // printf("pid_str: %s\n", pid_str);
    // printf("signal_str: %s\n", signal_str);

    if(pid_str == NULL || signal_str == NULL){
        printf("ERROT! Usage: ping <pid> <signal_number>\n");
        return;
    }

    pid_t pid = atoi(pid_str);
    int sig = atoi(signal_str) % 32;

    // printf("pid: %d\n", pid);
    // printf("sig: %d\n", sig);

    if(kill(pid, 0) == -1){
        printf("No such process found\n");
        return;
    }

    if(kill(pid, sig) == -1){
        perror("Error sending signal");
    }else{
        printf("Sent signal %d to process (%d)\n", sig, pid);
    }
}

double redirect(char *cmd, int bg){
    int saved_stdout = -1;
    int saved_stdin = -1;
    char *inp = strchr(cmd, '<');
    char *out = strchr(cmd, '>');

    char *input_file = NULL, *output_file = NULL;

    if(inp != NULL){
        input_file = inp+1;
        *inp = '\0';
    }
    if(out != NULL){
        output_file = out+1;
        *out = '\0';
    }
    int fd_out;
    if(output_file != NULL){
        output_file = trimm(output_file);
        if(*output_file == '>'){
            output_file++;
            output_file = trimm(output_file);
            fd_out = open(output_file, O_WRONLY | O_APPEND);
            if(fd_out == -1){
                perror("Error opening file");
                return 1;
            }
            saved_stdout = dup(1);
            dup2(fd_out, 1);
        }else{
            fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(fd_out == -1){
                perror("Error opening file");
                return 1;
            }
            saved_stdout = dup(1);
            dup2(fd_out, 1);
        }
    }
    int fd_in;
    if(input_file != NULL){
        input_file = trimm(input_file);
        fd_in = open(input_file, O_RDONLY);
        if(fd_in == -1){
            perror("Error opening file");
            return 1;
        }
        saved_stdin = dup(0);
        dup2(fd_in, 0);
    }
    // printf("%s\n", input_file);
    // printf("%s\n", output_file);
    double elapsed = execute(cmd, bg);

    if(saved_stdout != -1){
        dup2(saved_stdout, 1);
        close(saved_stdout);
        close(fd_out);
    }
    if(saved_stdin != -1){
        clearerr(stdin); // ?
        dup2(saved_stdin, 0);
        close(saved_stdin);
        close(fd_in);
    }
    return elapsed;
}


double execute(char *cmdd, int bg){
    char *cmd = (char *)malloc(strlen(cmdd) + 1);
    strcpy(cmd, cmdd);
    cmd = trimm(cmd);

    if(strlen(cmd) == 0) return 0;

    struct timespec start, end;
    if(!bg){
        clock_gettime(CLOCK_MONOTONIC, &start); // CLOCK_REALTIME or no?
    }

    // check_bg_processes(); // ?
    char *tmp = (char *)malloc(strlen(cmd) + 1);
    strcpy(tmp, cmd);
    char *command = strtok(tmp, " ");
    if(command == NULL){
        return 0;
    }
    if(strcmp(command, "hop") == 0){
        hop_cmd(cmd);
        return 0;
    }
    if(strcmp(command, "reveal") == 0){
        reveal_cmd(cmd);
        return 0;
    }
    if(strcmp(command, "seek") == 0){
        seek_cmd(cmd);
        return 0;
    }
    if(strcmp(command, "log") == 0){
        char *log_cmd = strtok(cmd, " ");
        log_cmd = strtok(NULL, " ");

        if (log_cmd == NULL) {
            display_log();
        }else if (strcmp(log_cmd, "purge") == 0){
            purge_log();
        }else if (strcmp(log_cmd, "execute") == 0){
            char *index_str = strtok(NULL, " ");
            if(index_str != NULL){
                int index = atoi(index_str);
                double elapsed = execute_log_command(index);
                return elapsed;
            }else{
                printf("ERROR: Missing index for log execute\n");
            }
        }
        return 0;
    }
    if(strcmp(command, "proclore") == 0){
        command = strtok(NULL, " ");
        proclore(command);
        return 0;
    }

    if(strcmp(command, "iMan") == 0){
        command = strtok(NULL, " ");
        // Handle (also if command is NULL)
        if(command == NULL){
            printf("ERROR: Missing command for iMan\n");
            return 0;
        }
        iMan(command);
        return 0;
    }

    if(strcmp(command, "neonate") == 0){
        command = strtok(NULL, " ");
        if(command == NULL){
            printf("ERROR: Missing argument for neonate\n");
            return 0;
        }
        if(strcmp(command, "-n") != 0){
            printf("ERROR: Invalid argument for neonate\n");
            return 0;
        }
        command = strtok(NULL, " ");
        if(command == NULL){
            printf("ERROR: Missing time argument for neonate\n");
            return 0;
        }
        int t = atoi(command);
        if(t < 0){
            printf("ERROR: Invalid time argument for neonate\n");
            return 0;
        }
        neonate(t);
        return 0;
    }
    if(strcmp(command, "activities") == 0){
        print_activities();
        return 0;
    }
    if(strcmp(command, "ping") == 0){
        ping_command(cmd);
        return 0;
    }
    if (strcmp(command, "fg") == 0) {
        char *pid_str = strtok(NULL, " ");
        if (pid_str == NULL) {
            printf("Usage: fg <pid>\n");
            return 0;
        }
        pid_t pid = atoi(pid_str);
        double elapsed = fg(pid);
        return elapsed;
    }
    if (strcmp(command, "bg") == 0) {
        char *pid_str = strtok(NULL, " ");
        if (pid_str == NULL) {
            printf("Usage: bg <pid>\n");
            return 0;
        }
        pid_t pid = atoi(pid_str);
        bg_func(pid);
        return 0;
    }

    pid_t pid = fork();
    if(pid < 0){
        perror("ERROR");
    }else if(pid == 0){
        setpgid(0, 0);
        if(!bg){
            tcsetpgrp(STDIN_FILENO, getpid());
        }
        int i = 0;
        char *args[64];
        char cmd_copy[256];
        strncpy(cmd_copy, cmd, sizeof(cmd_copy));
        args[0] = strtok(cmd_copy, " ");
        while(args[i] != NULL){
            args[++i] = strtok(NULL, " ");
        }
        args[i] = NULL;
        
        if(execvp(args[0], args) < 0){
            printf("ERROR: Command not found\n");
            exit(EXIT_FAILURE);
        }
        exit(0);
        
    }else{
        setpgid(pid, pid);
        if(!bg){
            add_process(command, pid);
            fg_pid = pid;
            fg_running = 1;
            tcsetpgrp(STDIN_FILENO, pid);
            int status;
            waitpid(pid, &status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, getpgrp());
            if(WIFSTOPPED(status)){
                stop_foreground_process(pid);
            }else if(WIFEXITED(status) || WIFSIGNALED(status)){
                remove_terminated_process(pid);
            }
            fg_pid = 0;
            fg_running = 0;
            if(WIFEXITED(status) || WIFSIGNALED(status)){
                remove_terminated_process(pid);
            }


            clock_gettime(CLOCK_MONOTONIC, &end);
            double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            return elapsed;
        }else{
            // int status;
            // waitpid(pid, &status, WNOHANG);
            // if(WIFEXITED(status) || WIFSIGNALED(status)){}
            // else{
                add_process(command, pid);
                printf("[%d] %d\n", process_count, pid);
            // }
            // erroneous commands should not go backgroudn...! WHY IS THIS NOT WORKING
            // The shell still looks shabby because of the error message
        }
    }
}

void sigint_handler(int signum){
    if(fg_pid > 0){
        kill(-fg_pid, SIGINT); // - means full process group
    }
}

void sigtstp_handler(int signum){
    if(fg_pid > 0){
        kill(-fg_pid, SIGTSTP);
        stop_foreground_process(fg_pid);
        fg_running = 0;
        fg_pid = 0;
        tcsetpgrp(STDIN_FILENO, getpgrp());
    }
}
