#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include "prompt.h"
#include "command.h"
#include "log.h"
#include "activities.h"
#include<signal.h>
#include "alias.h"
#include<termios.h>
#include<sys/types.h>
#include<sys/wait.h>

// Background processes with piping! The output which includes the pid of bg process gets redirected if applicable!

char home[LEN];
extern volatile sig_atomic_t fg_pid;
extern volatile sig_atomic_t fg_running;
extern int process_count;
extern ProcessInfo processes[LEN];

int main(){
    setpgid(0, 0);
    tcsetpgrp(STDIN_FILENO, getpgrp());

    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGTTOU, SIG_IGN); // to ignore
    signal(SIGTTIN, SIG_IGN); // to ignore
    
    getcwd(home, sizeof(home));  // Initialise home based on where shell is called from first!
    init_aliases(home);
    double elapsed = 0;
    char tmp[LEN];
    while(1){
        prompt(elapsed, tmp);
        elapsed = 0;
        char input[LEN];

        if(fgets(input, sizeof(input), stdin) == NULL){
            printf("\nLogging out\n");
            break;
        }
        input[strcspn(input, "\n")] = 0;
        char *for_log = strdup(input);
        int to_log = 1;
        // logadd(input);
        trimm(input);
        strcpy(tmp, input);

        // ;
        char *semicolon_commands[LEN];
        int semicolon_count = 0;
        semicolon_commands[0] = strtok(input, ";");
        while(semicolon_commands[semicolon_count] != NULL){
            semicolon_commands[++semicolon_count] = strtok(NULL, ";");
        }

        for(int i = 0; i < semicolon_count; i++){
            char *piped_commands[LEN];
            int pipe_count = 0;
            semicolon_commands[i] = trimm(semicolon_commands[i]);
            if(semicolon_commands[i][strlen(semicolon_commands[i]) - 1] == '|'){
                printf("Invalid use of pipe\n");
                continue;
            }
            // piping
            piped_commands[0] = strtok(semicolon_commands[i], "|");
            while(piped_commands[pipe_count] != NULL){
                piped_commands[++pipe_count] = strtok(NULL, "|");
            }

            if(pipe_count > 1){
                int flag_idk = 0;
                for(int j = 0; j < pipe_count; j++){
                    if(strlen(trimm(piped_commands[j])) == 0){
                        flag_idk = 1;
                        break;
                    }
                }
                if(flag_idk){
                    printf("Invalid use of pipe\n");
                    continue;
                }
            }

            if(pipe_count == 1){
                // normal, now split on & too
                char *cmd = trimm(piped_commands[0]);
                char *final_commands[LEN];
                int final_count = 0;
                int flag = 0;
                int bg_f[LEN] = {0};
                if(piped_commands[0][strlen(piped_commands[0]) - 1] == '&'){
                    flag = 1;
                    piped_commands[0][strlen(piped_commands[0]) - 1] = '\0';
                }
                char *tmpp = strtok(piped_commands[0], "&");
                tmpp = trimm(tmpp);
                while(tmpp != NULL){
                    final_commands[final_count] = tmpp;
                    final_count++;
                    tmpp = strtok(NULL, "&");
                    if(tmpp != NULL || flag){
                        bg_f[final_count - 1] = 1;
                    }
                }
                for(int j = 0; j < final_count; j++){
                    char *cmd_final = trimm(final_commands[j]);
                    int bg = bg_f[j];
                    if(strlen(cmd_final) == 0) continue;
                    char *alias_c = check_alias(cmd_final);
                    if(alias_c != NULL){
                        // strcpy(cmd_final, alias_c);
                        cmd_final = alias_c;
                    }
                    if(strcmp(cmd_final, "log") == 0 || strncmp(cmd_final, "log ", 4) == 0){
                        to_log = 0;
                    }
                    if(!bg){
                        fg_pid = 0;
                        fg_running = 1;
                        elapsed += redirect(cmd_final, bg);
                        fg_pid = 0;
                        fg_running = 0;
                    }else{
                        redirect(cmd_final, bg);
                    }
                }
            }else{
                // pipeline, check for & still
                int pipes[pipe_count - 1][2];
                for(int j = 0; j < pipe_count - 1; j++){
                    if(pipe(pipes[j]) < 0){
                        perror("pipe");
                        exit(1);
                    }
                }

                for(int j = 0; j < pipe_count; j++){
                    char *cmd = trimm(piped_commands[j]);
                    int bg = 0;
                    if(cmd[strlen(cmd) - 1] == '&'){
                        bg = 1;
                        cmd[strlen(cmd) - 1] = '\0';
                    }
                    char *alias_c = check_alias(cmd);
                    if(alias_c != NULL){
                        // strcpy(cmd, alias_c);
                        cmd = alias_c;
                    }
                    if(strcmp(cmd, "log") == 0 || strncmp(cmd, "log ", 4) == 0){
                        to_log = 0;
                    }
                    pid_t pid = fork();
                    if(pid == 0){
                        if(j > 0){
                            dup2(pipes[j-1][0], STDIN_FILENO);
                        }
                        if(j < pipe_count - 1){
                            dup2(pipes[j][1], STDOUT_FILENO);
                        }
                        for(int k = 0; k < pipe_count - 1; k++){
                            close(pipes[k][0]);
                            close(pipes[k][1]);
                        }
                        redirect(cmd, bg);
                        exit(0);
                    }
                }

                for(int j = 0; j < pipe_count - 1; j++){
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // Wait for all child processes otherwise how pipe ? (cz concurrent lwk)
                for(int j = 0; j < pipe_count; j++){
                    wait(NULL);
                }
            }
        }
        if(to_log){
            logadd(for_log);
        }
    }

    // kill
    for(int i = 0; i < process_count; i++){
        kill(processes[i].pid, SIGKILL);
    }

    return 0;
}
