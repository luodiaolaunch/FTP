#include "client.h"


int main(int argc, char *argv[])
{

	if(argc != 3)
		err_quit("error args:EXE IP PROT");
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		err_quit("socket error");

	struct sockaddr_in saddr;
	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(atoi(argv[2]));
	saddr.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(sockfd, (struct sockaddr*) &saddr, sizeof(saddr)) == -1)
		err_quit("connect error");

	while(0  != sys_login(sockfd))
	{
		printf("Access denied\n");
	}
	int ret, len;
	char line_buf[LINE_LEN];
	while(1)
	{
		//接收前台命令
		bzero(line_buf, sizeof(line_buf));
		printf(">");
		fflush(stdin);
		if(fgets(line_buf, sizeof(line_buf), stdin) == NULL)
			err_quit("fgets error");
		len = strlen(line_buf);
		line_buf[len-1] = 0;//去除行尾添加的换行符
		len--;
	//	printf("len:%d,buf:%s\n", len,line_buf);
		send_n(sockfd, (char*)&len, sizeof(int));
		send_n(sockfd, line_buf, len);
		handle_cmd_line(sockfd, line_buf, len);
	}
//	return 0;
}

int sys_login(int sockfd)
{
	char usr_name[USR_NAME_LEN] = {0};
	char pwd_buf[PASSWD_LEN] = {0};
	int len;
	int login = 0;


	//发送用户密码
	printf("Account :");
	fflush(stdin);
	if(NULL == fgets(usr_name, sizeof(usr_name), stdin))
	{
		perror("fgets usr_name");
		return -1;
	}
	len = strlen(usr_name);
	usr_name[len-1] = 0;
	len--;
	send_n(sockfd, (char*) &len, sizeof(int));
	send_n(sockfd, usr_name, len);

	printf("Password:");
	fflush(stdin);
	if(NULL == fgets(pwd_buf, sizeof(pwd_buf), stdin))
	{
		perror("fgets pwd_buf");
		return -1;
	}
	len = strlen(pwd_buf);
	pwd_buf[len-1] = 0;
	len--;
	send_n(sockfd, (char*) &len, sizeof(int));
	send_n(sockfd, pwd_buf, len);

	//接收服务端验证标志
	recv_n(sockfd, (char*)&login, sizeof(int));
	if(login)
		return 0;
	else
		return -1;
}
void recv_file(int sockfd)
{
	int fd, ret, renum, is_dir;
	char buf[MAX_SIZE] = {0};


	//目录标志	
	recv_n(sockfd, (char*) &is_dir, sizeof(int));
	if(is_dir)
	{
		printf("can't gets directory\n");
		return;
	}
	if(recv(sockfd, &renum, sizeof(int), 0) == -1)
		err_quit("recv file name lenth error");
	if(recv(sockfd, buf, renum, 0) == -1)
		err_quit("recv file name error");
	fd = open(buf, PEER, FILE_MODE);
	if(fd == -1)
	{
		perror("creat file error");
		return;
	}
	ret = 0; 
//	printf("downloading..\n");
	
	unsigned long long cnt = 0;
	unsigned long file_size;
	int get = 0;
	ret = recv_n(sockfd, (char*)&file_size, sizeof(file_size));
	if(ret != sizeof(file_size))
		err_quit("recv file_size error");

	while(1)
	{
		bzero(buf, sizeof(buf));
		ret = recv(sockfd, &renum, sizeof(int), 0);
		
		//ret = 0服务端关闭
		if(ret == 0)
		{
			printf("\ndisconnected\n");
			close(sockfd);
			close(fd);
			exit(-1);
		}
		if(renum == 0)
			break;
		if(renum <= 1020 && renum > 0)
		{
			ret = recv_n(sockfd, buf, renum);
			//ret = 0服务端关闭
			if(ret == 0)
			{
				printf("\ndisconnected\n");
				close(sockfd);
				close(fd);
				exit(-1);
			}

			if(ret != renum)
				err_quit("recv file data error");
			if(write(fd, buf, renum) == -1)
				err_quit("write file error");
			cnt += renum;
			get = (int)((100*cnt)/file_size);
			printf("\rreceive file data:%d%%", get);
			
		}
	}
	close(fd);
//	printf("\nreceive %llu bytes\nreceive suceess!\n", cnt);
	printf("\n");
}

void err_quit(char *str)
{
	fprintf(stderr, "%s\n", str);
	exit(-1);
}			

//处理前台命令
void handle_cmd_line(int sockfd, char *buf, int len)
{
	
	int i,ret, file_cnt, cmd_num, opendir_sucs = 1;
	char cmd0_buf[CMD_LEN], cmd1_buf[CMD_LEN];
	char *recv_buf;
	cmd_num = sscanf(buf, "%s %s", cmd0_buf, cmd1_buf);
	if(cmd_num < 1)
	{
		printf("cmd_num:%d\n", cmd_num);
		perror("sscanf cmd0_buf error");
		return;
	}

	//处理客户端命令
	if(!strcmp(cmd0_buf, "ls"))
	{
		//接收 打开文件标记
		recv_n(sockfd, (char*) &opendir_sucs, sizeof(int));
		if(opendir_sucs == 0)
		{
			printf("opendir error\n");
			return;
		}
		//接收文件数
		if(recv_n(sockfd, (char*) &file_cnt, sizeof(int)) != 4)
		{
			perror("recv_n file_cnt");
			return;
		}

		//接收ls显示
		if((recv_buf = calloc(file_cnt, CMD_LEN)) == NULL)
		{
			perror("calloc recv_buf error");
			return;
		}
	}else if(!strcmp(cmd0_buf, "pwd"))
	{
		if((recv_buf = malloc(PATH_LENTH)) == NULL)
		{
			perror("pwd malloc recv_buf error");
			return;
		}
	}else if(!strcmp(cmd0_buf, "gets"))
	{
		if(cmd_num <= 1)
		{
			printf("gets: missing operand\n");
			return;
		}
		recv_file(sockfd);	
		return;
	}else if(!strcmp(cmd0_buf, "puts"))
	{
		if(cmd_num <= 1)
		{
			printf("puts: missing operand\n");
			return;
		}
		send_file(sockfd, cmd1_buf);
		return;
	}else if(!strcmp(cmd0_buf, "remove"))
	{
		if(cmd_num <= 1)
		{
			printf("remove: missing operand\n");
			return;
		}
		recv_n(sockfd, (char*) &ret, sizeof(int));
		if(ret == 0)
			printf("remove: Permission denied\n");
		return;	
	}else 
		goto endtag;

	//接收并显示到处理后的信息
	if(-1 == recv_operate_data(sockfd, recv_buf, strlen(recv_buf)))
	{
		perror("send_operate_data error");
		free(recv_buf);
		recv_buf = NULL;
		return;
	}
	bzero(recv_buf, strlen(recv_buf));
	
endtag:
	free(recv_buf);
	recv_buf = NULL;
}

int recv_operate_data(int sockfd, char *buf, int len)
{
	int ret;
//	printf("enter recv_operate\n");
	if(buf == NULL)
		return -1;
	int renum;
	if(4 != recv_n(sockfd, (char *) &renum, sizeof(int)))
	{
		perror("recv_n recv_buf length error");
		return -1;
	}
//	printf("the renum %d\n", renum);
	bzero(buf,sizeof(buf));
	ret = recv_n(sockfd, buf, renum);
	if(ret == renum+100)
	{
		perror("recv_n recv_buf error");
		return -1;
	}
//	printf("ret :%d\n", ret);
	printf("%s", buf);
	return 0;
}

//上载文件功能
void send_file(int sockfd, char *file_name)
{
	if(file_name == NULL)
	{
		printf("file_name: NULL");
		return;
	}
	MSG msg;
	int ret;
	int fd;
	int end_flag = 0;
	unsigned long long cnt = 0;
	unsigned long file_size;
	struct stat fst;
	int get = 0;
	bzero(&msg, sizeof(msg));
	//printf("the file name msg.len:%d\n", msg.len);
	fd = open(file_name, O_RDONLY);
	if(fd == -1)
	{	
		printf("open file error\n");
		return;
	}
//	else
//		printf("open success");
	if(fstat(fd, &fst) == -1)
		err_quit("fstat error");
	int i = 0, is_dir = 0;
	if(S_ISDIR(fst.st_mode))
	{
		printf("can't puts directory\n");
		close(fd);
		return;
	}
	file_size = fst.st_size;
	send(sockfd, &file_size, sizeof(file_size), 0);
	//printf(", the file size is %ld bytes\n", file_size);
	while(bzero(&msg, sizeof(msg)), (msg.len = read(fd, msg.buf, sizeof(msg.buf))) > 0)
	{
		send_n(sockfd,(char*)&msg, sizeof(int)+msg.len);

		cnt += msg.len;
		get = (int)((100*cnt)/file_size);
		printf("\rsend file data:%d%%", get);
	}
	send(sockfd, &end_flag, sizeof(int), MSG_CONFIRM);
//	printf("\nsend file success\n");
	printf("\n");
	close(fd);
}


