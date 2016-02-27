#ifndef __HEAD_H__
#define __HEAD_H__

#include <stdio.h>
#include <stdlib.h>
//#define _XOPEN_SOURCE
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/epoll.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <syslog.h>
#include <shadow.h>
#define _GUN_SOURCE
#include <crypt.h>
#include <pwd.h>
#include "msg.h"
#include "file_operate.h"

void err_quit(char *str);
void err_sys(char *str);
void *con_handle(void *arg);
void send_n(int sockfd, char *buf, int len);
int recv_n(int sockfd, char* p, int len);
int initsever(int type, struct sockaddr *addr, socklen_t alen, int qlen);

int check_passwd(int sockfd, char *usr_name);
void get_salt(char *salt, char *passwd);
void mode_format(mode_t mode, char *buf, int len);
void time_format(char *buf, int len);
void handle_client_cmd(int sockfd, char *cmdbuf[], int *num);
void handle_request(int sockfd, struct sockaddr_in *addr_info, char* usr_name);
int check_remove_rights(char *file_name, char *usr_name);

#endif

