#ifndef __WORKQUE_H__
#define __WORKQUE_H__
#include "head.h"

typedef struct tag_node{
	int sockfd;
	struct tag_node *nd_next;
}node_t, *pnode_t;
typedef struct tag_que{
	pnode_t head, tail;
	int capacity;
	int size;
	pthread_mutex_t que_mutex;
}que_t, *pque_t;

void ft_que_init(pque_t pq, int capacity);
void ft_que_set(pque_t pq, pnode_t pnew);
void ft_que_get(pque_t pq, pnode_t *pget);
void ft_que_destroy(pque_t pq);
int  ft_que_full(pque_t pq);
int  ft_que_empty(pque_t pq);

#endif
