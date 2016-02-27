#ifndef __MSG_H__
#define __MSG_H__

#define MAX_SIZE		 1024
#define PEER 			 O_WRONLY|O_CREAT
#define FILE_MODE        0644
#define LINE_LEN         1024
#define CMD_LEN          256
#define CMD_NUM          5
#define PATH_LENTH		 1024
#define PTHREAD_NUM 	 10
#define QUEEN_NUM 	 	100
#define FILE_NAME 		 "test"
#define LS_L        	  1
#define LS          	  0
#define USR_NAME_LEN	  32
#define PASSWD_LEN    	  32

typedef struct tag{
	int len;
	char buf[MAX_SIZE - sizeof(int)];
}MSG, pMSG;


#endif

