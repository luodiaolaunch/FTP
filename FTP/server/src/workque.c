#include "workque.h"

void ft_que_init(pque_t pq, int capacity)
{
	pq->head = NULL;
	pq->tail = NULL;
	pq->capacity = capacity;
	pq->size = 0;
	if(pthread_mutex_init(&pq->que_mutex, NULL) == -1)
		err_quit("pthread_mutex_init error");
}

void ft_que_set(pque_t pq, pnode_t pnew)
{
	if(pnew == NULL)
		return;
	if(pq->tail != NULL)
	{
		pq->tail->nd_next = pnew;
		pq->tail = pnew;
	}
	else
		pq->head = pq->tail = pnew;
	pq->size++;
}

void ft_que_get(pque_t pq, pnode_t *pget)
{
	*pget = pq->head;
	pq->head = pq->head->nd_next;
	if(pq->head == NULL)
		pq->tail = NULL;
	pq->size--;
}

int  ft_que_full(pque_t pq)
{
	return (pq->size == QUEEN_NUM);
}

int ft_que_empty(pque_t pq)
{
	return(pq->size == 0);
}
