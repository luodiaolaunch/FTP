#include "factory.h"

int main(int argc, char *argv[])
{
	if(argc != 2)
		err_quit("error args");
	
	//读取 IP PORT
	FILE* conf_fp;
	conf_fp = fopen(argv[1], "r");
	if(conf_fp == NULL)
		err_quit("fopen server.conf");	
	char ip_buf[LINE_LEN];
	int port_value;
	bzero(ip_buf, sizeof(ip_buf));

	if(1 != fscanf(conf_fp, "ip = %s\n", ip_buf))
	{
		fclose(conf_fp);
		err_quit("fscanf ip");
	}
	if(1 != fscanf(conf_fp, "port = %d", &port_value))
	{
		fclose(conf_fp);
		err_quit("fscanf port");
	}
	fclose(conf_fp);

	//初始化，启动线程
	factory_t my_factory;
	factory_init(&my_factory, PTHREAD_NUM, 0, con_handle, NULL, QUEEN_NUM);
	factory_start(&my_factory);
	
	//初始化套接字
	struct sockaddr_in saddr;
	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port_value);
	saddr.sin_addr.s_addr = inet_addr(ip_buf);
	int sockfd;
	sockfd = initsever(SOCK_STREAM, (struct sockaddr*) &saddr,\
		sizeof(saddr), QUEEN_NUM);
	if(sockfd == -1)
	{
		perror("initsevr error");
		exit(-1);
	}

	printf("waiting client connecting..\n");

	//处理客户端请求
	int cli_fd;
	pque_t pq = my_factory.pque;


	struct sockaddr_in cli_addr;
	int sockaddr_len;
	while(1)
	{
		//!创建新节点接收客户端sockfd， 在相应线程中释放节点
		pnode_t pnode = malloc(sizeof(node_t));
		if(pnode == NULL)
			err_quit("malloc pnode");
		pnode->nd_next = NULL;
		bzero(&cli_addr,sizeof(cli_addr));
		sockaddr_len = sizeof(cli_addr);
		cli_fd = accept(sockfd, (struct sockaddr*)&cli_addr, &sockaddr_len);
		if(cli_fd == -1)
		{
			printf("accept error\n");
			continue;
		}
		
		pthread_mutex_lock(&pq->que_mutex);
		if(ft_que_full(pq))
			continue;
		pnode->sockfd = cli_fd;
		ft_que_set(pq, pnode);
		memcpy(my_factory.cli_info, &cli_addr, sizeof(cli_addr));
		
		pthread_mutex_unlock(&pq->que_mutex);
		pthread_cond_signal(&my_factory.cond);
	//	printf("start processing....\n");
	}
	return 0;
}

int initsever(int type, struct sockaddr *saddr, socklen_t alen, int qlen)
{
	int err, sockfd;
	int reuse = 1;
	if((sockfd = socket(((struct sockaddr_in*)saddr)->sin_family, type, 0)) == -1)
	{
		return -1;
	}
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
	{
		goto errout;
	}
	if(bind(sockfd, saddr, alen) == -1)
	{
		goto errout;
	}
	if(type == SOCK_STREAM || type == SOCK_SEQPACKET)
		if(listen(sockfd, qlen) == -1)
		{
			goto errout;
		}
	return sockfd;

errout:
	err = errno;
	close(sockfd);
	errno = err;
	return -1;
}

//线程处理函数 
void *con_handle(void *arg)
{	
	pfactory_t pf = (pfactory_t) arg;
	pque_t pq = pf->pque;
	pnode_t pnode;
	int sockfd;
	int i,ret;
	struct sockaddr_in *addr_info;
	int login = 1;
	char usr_name[USR_NAME_LEN];

	//存放客户端地方信息
	if((addr_info = calloc(1, sizeof(struct sockaddr_in))) == NULL)
	{
		perror("con_handle calloc");
		return NULL;
	}

	//文件操作处理
	while(1)
	{
		bzero(addr_info, sizeof(addr_info));
		pthread_mutex_lock(&pq->que_mutex);
		while(ft_que_empty(pq))
			pthread_cond_wait(&pf->cond, &pq->que_mutex);
		ft_que_get(pq, &pnode);
		//在加锁时拷贝一份客户端信息： cli_info在新的客户端请求时会被重新赋值
		memcpy(addr_info, pf->cli_info, sizeof(struct sockaddr_in));
		pthread_mutex_unlock(&pq->que_mutex);
		sockfd = pnode->sockfd;

		//释放从队列里取出的节点
		free(pnode);
		pnode = NULL;
	
		//密码验证功能
		do{
			ret = check_passwd(sockfd, usr_name);

			//发送验证通过/失败标志
			if(ret == 0)
				login = 1;
			else
				login = 0;
			send_n(sockfd, (char*) &login, sizeof(int));

			//客户端关闭 外层while继续循环处理
			if(ret == -2)
			{
				close(sockfd);
				break;
			}
		}while(ret != 0);

		if(ret == -2)
			continue;

		//记录登录日志
		syslog(LOG_INFO, "client ip:%s  port:%d operand:connect\n",\
			inet_ntoa(addr_info->sin_addr), ntohs(addr_info->sin_port));

		printf("client ip:%s  port:%d operand:connect\n",\
			inet_ntoa(addr_info->sin_addr), ntohs(addr_info->sin_port));	

		
		//process mask...
		while(1)
		{
			handle_request(sockfd, addr_info, usr_name);
		}

	}

endtag:
	close(sockfd);
	free(addr_info);
	addr_info = NULL;
}

void get_salt(char *salt, char *passwd)
{
	int i , j;
	for(i = 0, j = 0; passwd[i] && j != 3; ++i)
	{
		if(passwd[i] == '$')
			++j;
	}
	strncpy(salt, passwd, i-1);
}

//处理客户端相关功能
void handle_request(int sockfd, struct sockaddr_in *addr_info, char* usr_name)
{

	char *ret_buf = NULL;
	int i;
	char *cmd_buf[CMD_LEN];
	int cmd_valid = 1;
	int cmd_invalid = 0;
	int rm_rights = 0;

	//存放客户端命令行buffer
	for(i = 0; i < CMD_NUM; i++)
	{
		if((cmd_buf[i] = calloc(1, CMD_LEN)) == NULL)
		{
			perror("calloc cmd_buf");
			return;
		}
	}
	int cmd_num;
	handle_client_cmd(sockfd, cmd_buf, &cmd_num);
//	printf("cmd_num[0]:%s\n", cmd_buf[0]);
	if(cmd_buf[0] == NULL)
	{
		err_sys("recv cmd data error");
		cmd_valid = 0;
		goto endtag;
	}

	if(!strcmp(*cmd_buf, "ls"))
	{

		if(cmd_num == 1) //ls 无参数
		{
		//	printf("cmd_num:1\n");
			ret_buf = file_ls(NULL, LS_L, sockfd);
		}
		else
			ret_buf = file_ls(cmd_buf[1], LS_L, sockfd);
		if(ret_buf == NULL)
			goto endtag;
	}
	else if(!strcmp(*cmd_buf, "pwd"))
	{
		if((ret_buf = calloc(1, PATH_LENTH)) == NULL)
		{
			err_sys("malloc pwd ret_buf error");
			goto endtag;
		}
		
		if(-1 == file_pwd(sockfd, ret_buf, PATH_LENTH))
			goto endtag;
		//路径名后加换行符
		ret_buf[strlen(ret_buf)] = '\n';
		ret_buf[strlen(ret_buf)] = 0;
	}else if(!strcmp(*cmd_buf, "gets"))
	{
		if(cmd_num != 2)
		{
		//	printf("gets: missing operand\n");
			goto endtag;
		}
		send_file(sockfd, cmd_buf[1]);
		goto endtag; //加上此句 不需要执行send_operate_data 若ret_buf 没初始化为NULL产生段错误
	}else if(!strcmp(*cmd_buf, "remove"))
	{
		if(cmd_num <= 1)
		{
		//	printf("remove: missing operand\n");
			goto endtag;
		}
		if(0 == check_remove_rights(cmd_buf[1], usr_name))
		{
			send_n(sockfd, (char*) &cmd_valid, sizeof(int));
			file_remove(cmd_buf, cmd_num);
		}
		else
			send_n(sockfd, (char*) &cmd_invalid, sizeof(int));
		//	printf("remove: Permission denied\n");
		goto endtag;
	}else if(!strcmp(*cmd_buf, "cd"))
	{
		if(cmd_num == 1)
			if(-1 == file_pwd(sockfd, cmd_buf[1], CMD_LEN))
			{
				printf("file_pwd error\n");
				goto endtag;
			}
		file_cd(cmd_buf, cmd_num);
		goto endtag;
	}else if(!strcmp(*cmd_buf, "puts"))
	{
		if(cmd_num != 2)
		{
		//	printf("puts: missing operand\n");
			goto endtag;
		}
		recv_file(sockfd, cmd_buf[1]);
		goto endtag;

	}else		
	{
		cmd_valid = 0;
		goto endtag;
	}
	if(0 != send_operate_data(sockfd, ret_buf, strlen(ret_buf)))
		goto endtag;

endtag:
	free(ret_buf);
	ret_buf = NULL;

	if(cmd_valid)//命令有效被执行 才写进日志
		syslog(LOG_INFO, "client ip:%s  port:%d operand:%-8s\n", inet_ntoa(addr_info->sin_addr),\
				ntohs(addr_info->sin_port),cmd_buf[0]);
	for(i = 0; i < CMD_NUM; i++)
	{
		free(cmd_buf[i]);
		cmd_buf[i] = NULL;
	}
}

//处理客户端请求命令行内容
void handle_client_cmd(int sockfd, char *cmdbuf[], int *num)
{
	char recv_buf[LINE_LEN];
	int renum = 0;
	int ret;
	int i;
	bzero(recv_buf, sizeof(recv_buf)); // 不初始化会保留上次内容？
	for(i = 0; i < CMD_NUM; i++)
		bzero(cmdbuf[i], CMD_LEN);

	ret = recv_n(sockfd, (char*) &renum, sizeof(int));
	if(ret != 4)
	{
		perror("recv client cmd lenth error");
		return;
	}
	ret = recv_n(sockfd, recv_buf, renum);
	if(ret != renum)
	{
		perror("recv client cmd error");
		return;
	}
	ret = sscanf(recv_buf, "%s %s %s %s %s", cmdbuf[0], \
			cmdbuf[1], cmdbuf[2], cmdbuf[3], cmdbuf[4]);
	*(cmdbuf+ret) = NULL;//
	*num = ret;
}

void err_quit(char *str)
{
	fprintf(stderr, "%s\n", str);
	exit(-1);
}

void err_sys(char *str)
{
	fprintf(stderr,"%s\n", str);
}

int check_remove_rights(char *file_name, char *usr_name)
{
	struct passwd *passwd_info;
	passwd_info = getpwnam(usr_name);

	//有效ID
	uid_t euid = passwd_info->pw_uid;
	gid_t egid = passwd_info->pw_gid;

	struct stat fst;
	int fd = open(file_name, O_RDONLY);
	if(fd == -1)
	{
		perror("open file");
		return -1;
	}
	if(-1 ==fstat(fd, &fst))
	{
		perror("fsat");
		return -1;
	}

	uid_t owner_uid = fst.st_uid;
	uid_t owner_gid = fst.st_gid;
	

	//root用户ID
	if(owner_gid != 0 && euid ==0)
		return 0;

	//一般用户ID
	if(euid == owner_uid)
		if((fst.st_mode & S_IWUSR) == S_IWUSR)
			return 0;
	else if(egid == owner_gid)
		if((fst.st_mode & S_IWGRP) == S_IWGRP)
			return 0;
	else
		if((fst.st_mode & S_IWOTH) == S_IWOTH)
			return 0;

	return -1;
}
