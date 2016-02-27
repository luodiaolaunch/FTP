#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include "msg.h"

int sys_login(int sockfd);
void recv_file(int sockddfd);
void err_quit(char *str);
void send_n(int sockfd, char*p, int len);
int recv_n(int sockfd, char* p, int len);
void handle_cmd_line(int sockfd, char *buf, int len);
int recv_operate_data(int sockfd, char *buf, int len); 
void send_file(int sockfd, char* file_name);
#endif
