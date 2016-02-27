#include "head.h"

//上载文件功能
void send_file(int sockfd, char *file_name)
{
	if(file_name == NULL)
	{
		printf("file_name: NULL");
		return;
	}
	int renum;
	MSG msg;
	int ret;
	int fd;
	int end_flag = 0;
	int is_dir = 0;
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
	//发送下载文件是否为目录标记
	if(fstat(fd, &fst) == -1)
		err_quit("fstat error");
 	if(S_ISDIR(fst.st_mode))
		is_dir = 1;
	else
		is_dir = 0;
	send_n(sockfd, (char*) &is_dir, sizeof(int));

	int len = strlen(file_name);
	send_n(sockfd, (char*) &len, sizeof(int));
	send_n(sockfd, file_name, len);
	int i =0;
	file_size = fst.st_size;
	send(sockfd, &file_size, sizeof(file_size), 0);
	//printf(", the file size is %ld bytes\n", file_size);
	while(bzero(&msg, sizeof(msg)), (msg.len = read(fd, msg.buf, sizeof(msg.buf))) > 0)
	{
//		printf("the len:%d\n",msg.len);
		send_n(sockfd,(char*)&msg, sizeof(int)+msg.len);
		
		cnt += msg.len;
		get = (int)((100*cnt)/file_size);
	//	printf("\rsend file data:%d%%", get);
	}
//	printf("\n");
	send_n(sockfd, (char*) &end_flag, sizeof(int));
	close(fd);
}

void recv_file(int sockfd, char* file_name)
{
	int fd, ret, renum;
	char buf[MAX_SIZE] = {0};
	
	if((fd = open(file_name, O_WRONLY|O_CREAT, 0664)) == -1)
	{
		perror("open file_name");
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
		recv(sockfd, &renum, sizeof(int), 0);
	//	printf("ret:%d renum:%d       ", ret, renum);
		if(renum == 0)
			break;
		if(renum <= 1020 && renum > 0)
		{
			ret = recv_n(sockfd, buf, renum);
			if(ret != renum)
				err_quit("recv file data error");
			if(write(fd, buf, renum) == -1)
				err_quit("write file error");
			cnt += renum;
			get = (int)((100*cnt)/file_size);
		//	printf("\rreceive file data:%d%%", get);
			
		}
	}
	close(fd);
//	printf("\n");
}


int  file_pwd(int sockfd, char *send_buf, int len)
{
	if(getcwd(send_buf, len) == NULL)
		return -1;
	return 0;
}


void file_remove(char *cmd_buf[], int cmd_num)
{
	int i;
	for(i = 1; i < cmd_num; i++)
		remove(cmd_buf[1]);	
}


void file_cd(char *cmd_buf[], int cmd_num)
{
	chdir(cmd_buf[1]);	
}


int send_operate_data(int sockfd, char *buf, int len)
{
	send_n(sockfd, (char*) &len, sizeof(int));
	send_n(sockfd, buf, len);
	return 0;
}


char* file_ls(char *cli_path_name, int flag, int sockfd)
{
	DIR *fdir;
	int file_cnt;
	int opendir_sus = 1;
	char path_name[USR_NAME_LEN];
	if(cli_path_name == NULL)
		strcpy(path_name, ".");
	else
		strcpy(path_name, cli_path_name);
	fdir=opendir(path_name);
	if(NULL==fdir)
	{
		opendir_sus = 0;
		send_n(sockfd, (char*)&opendir_sus, sizeof(int));
		return NULL;
	}
	send_n(sockfd, (char*)&opendir_sus, sizeof(int));
	struct dirent *p;
	char buf[1024];
	struct stat fst;
	int ret, len = 0;
	char* pdate;
	char parr[11];
	while(readdir(fdir) != NULL)
		file_cnt++;
	rewinddir(fdir);
	send_n(sockfd, (char*)&file_cnt, sizeof(int));
	char *send_buf = malloc(file_cnt*LINE_LEN);
	bzero(send_buf, file_cnt*LINE_LEN);
	while((p=readdir(fdir))!=NULL)
	{
		if(strcmp(p->d_name,".") && strcmp(p->d_name,".."))
		{
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%s/%s", path_name,p->d_name);
			ret=stat(buf,&fst);
			if(-1==ret)
			{
				perror("stat");
				exit(-1);
			}
			pdate=ctime(&fst.st_mtim.tv_sec);
			mdate(pdate);
			pmod(fst.st_mode,parr);
			sprintf(&send_buf[len], "%10s%2d%8s%8s%8ld%13s %-10s\n",parr,fst.st_nlink,getpwuid(fst.st_uid)->pw_name,getgrgid(fst.st_gid)->gr_name,fst.st_size,pdate+4,p->d_name);
			len = strlen(send_buf);
		//	printf("len:%d\n", len);
		}
	}
	return send_buf;
}

void mdate(char* p)
{
	char* pcur;
	pcur=p+strlen(p)-1;
	while(*pcur != ':')
	{
		pcur--;
	}
	*pcur='\0';
}
void pmod(mode_t mode,char* p)
{
	if(S_ISDIR(mode))
	{
		p[0]='d';
	}else{
		p[0]='-';
	}
	if(mode&S_IRUSR)
	{
		p[1]='r';
	}else{
		p[1]='-';
	}
	if(mode&S_IWUSR)
	{
		p[2]='w';
	}else{
		p[2]='-';
	}
	if(mode&S_IXUSR)
	{
		p[3]='x';
	}else{
		p[3]='-';
	}
	if(mode&S_IRGRP)
	{
		p[4]='r';
	}else{
		p[4]='-';
	}
	if(mode&S_IWGRP)
	{
		p[5]='w';
	}else{
		p[5]='-';
	}
	if(mode&S_IXGRP)
	{
		p[6]='x';
	}else{
		p[6]='-';
	}
	if(mode&S_IROTH)
	{
		p[7]='r';
	}else{
		p[7]='-';
	}
	if(mode&S_IWOTH)
	{
		p[8]='w';
	}else{
		p[8]='-';
	}
	if(mode&S_IXOTH)
	{
		p[9]='x';
	}else{
		p[9]='-';
	}
	p[10]='\0';
}		
