#ifndef LOG_H
#define LOG_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void load_log();
void save_log();
void logadd(const char *command);
void display_log();
void purge_log();
double execute_log_command(int index);

#endif
