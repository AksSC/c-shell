#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/utsname.h>
#include "prompt.h"
#include "command.h"
#include<pwd.h>

extern char home[LEN];

void prompt(double elapsed, char *cmd){
    char cwd[LEN];
    getcwd(cwd, sizeof(cwd));

    struct passwd *pw = getpwuid(getuid());
    char *username = pw->pw_name;
    struct utsname sys_info;
    uname(&sys_info);
    char *system_name = sys_info.nodename;
    if(elapsed > 2){
        if(strncmp(cwd, home, strlen(home)) == 0){
            printf("<%s@%s:~%s %s : %ds> ", username, system_name, cwd + strlen(home), cmd, (int)elapsed);
        }else{
            printf("<%s@%s:%s %s : %ds> ", username, system_name, cwd, cmd, (int)elapsed);
        }
    }else{
        if(strncmp(cwd, home, strlen(home)) == 0){
            printf("<%s@%s:~%s> ", username, system_name, cwd + strlen(home));
        }else{
            printf("<%s@%s:%s> ", username, system_name, cwd);
        }
    }
}
