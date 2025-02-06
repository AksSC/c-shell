#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<dirent.h>
#include<fcntl.h>
#include<string.h>
#include<ctype.h>
#include<termios.h>
#include<time.h>
#include "neonate.h"
#include<signal.h>
#include "command.h"

int get_latest(){
    FILE *fil = fopen("/proc/sys/kernel/ns_last_pid", "r"); // SURELY THIS IS ALLOWED
    if(!fil){
        perror("fopen");
        exit(1);
    }

    int ret;
    fscanf(fil, "%d", &ret);
    fclose(fil);

    return ret;
}


void non_blocking(int enable) {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if(enable){
        tty.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // non blocking
        signal(SIGTSTP, SIG_IGN); // ignore Ctrl-Z
        signal(SIGINT, SIG_IGN); // ignore Ctrl-C
    }else{
        tty.c_lflag |= (ICANON | ECHO); // reenable canonical mode and echo
        fcntl(STDIN_FILENO, F_SETFL, 0); // blocking
        signal(SIGTSTP, sigtstp_handler);
        signal(SIGINT, sigint_handler);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void neonate(int t) {
    non_blocking(1); // non-blocking
    char key_pressed;
    time_t last = time(NULL) - t;
    
    while(1){
        if(time(NULL) - last >= t){
            int pid = get_latest();
            printf("%d\n", pid);
            last = time(NULL);
        }
        key_pressed = getchar();
        if (key_pressed == 'x') {
            break;
        }
    }

    non_blocking(0); // restore blocking
}
