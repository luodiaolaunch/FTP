#include"head.h"

void send_n(int sockfd, char*p, int len)
{
	int send_sum, send_ret ;
	send_sum = 0;
	while(send_sum < len)
	{
		send_ret = send(sockfd, p + send_sum, len - send_sum, 0);
		if(-1 == send_ret)
		{
			if(errno == EINTR)
				continue;
			return;
		}
		send_sum += send_ret ;
	}
}
int recv_n(int sockfd, char* p, int len)
{
	int recv_sum, recv_ret ;
	recv_sum = 0 ;
	while(recv_sum < len)
	{
		recv_ret = recv(sockfd, p + recv_sum, len - recv_sum, 0);
	//	printf("recv_ret is %d\n",recv_ret);
		recv_sum += recv_ret ;
	}
	return recv_sum ;
}
