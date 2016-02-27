#ifndef __FILE_OPERATE_H__
#define __FILE_OPERATE_H__
#include "head.h"

char* file_ls(char *path_name, int flag, int sockfd);  
void send_file(int sockfd, char *file_name);
int  file_pwd(int sockfd, char *send_buf, int len);
void file_remove(char *cmd_buf[], int cmd_num);
void file_cd(char *cmd_buf[], int cmd_num);
void recv_file(int sockfd, char *file_name);
int send_operate_data(int sockfd, char *buf, int len);
void mdate(char *);
void pmod(mode_t ,char*);

#endif
