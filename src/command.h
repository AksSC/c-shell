#ifndef COMMAND_H
#define COMMAND_H

#define LEN 4096

double execute(char *cmd, int bg);
double redirect(char *cmd, int bg);
// void check_bg_processes();
char *trimm(char *str);
void sigint_handler(int signum);
void sigtstp_handler(int signum);

#endif