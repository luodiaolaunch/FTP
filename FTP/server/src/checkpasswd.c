#include "head.h"

int check_passwd(int sockfd, char *usr_buf)
{
	int renum;
	char usr_name[USR_NAME_LEN] = {0};
	char pwd_buf[PASSWD_LEN] = {0};
	struct spwd *sp;
	char salt[512] = {0};
	char *pcrypt;
	int ret;

	//获取客户端用户名密码
	ret = recv(sockfd,(char*) &renum, sizeof(int), 0);
	if(ret == 0)
		return -2;
	ret = recv(sockfd, usr_name, renum, 0);
	if(ret == 0)
		return -2;
	ret = recv(sockfd,(char*) &renum, sizeof(int), 0);
	if(ret == 0)
		return -2;
	ret = recv(sockfd, pwd_buf, renum, 0);
	if(ret == 0)
		return -2;

//	printf("the usrname:%s\npasswd:%s\n", usr_name, pwd_buf);
	//验证密码	
	sp = malloc(sizeof(struct spwd));
	if(sp == NULL)
	{
		perror("malloc sp");
		return -1;
	}
	if((sp = getspnam(usr_name)) == NULL)
	{
		perror("getspnam error");
		return -1;
	}

	get_salt(salt, sp->sp_pwdp);
	pcrypt = crypt(pwd_buf, salt);
	if(strcmp(sp->sp_pwdp,pcrypt) != 0)
	{
	//	printf("check failed\n");
		return -1;
	}
	strcpy(usr_buf, usr_name);
	return 0;
}


