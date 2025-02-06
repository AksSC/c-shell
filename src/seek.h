#ifndef SEEK_H
#define SEEK_H

void seek_directory(const char *target, char *base_path, int flag_d, int flag_f, int flag_e, int *found, char *single_match);
void seek_cmd(char *cmd);

#endif