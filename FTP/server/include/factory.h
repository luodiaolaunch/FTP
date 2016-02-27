#ifndef __FACTORY_H__
#define __FACTORY_H__

#include "head.h"
#include "workque.h"

typedef void* (*pfunc_t)(void*); //函数指针 + 指针函数
typedef struct tag_ft{
	pque_t pque;
	pthread_cond_t cond;
	pfunc_t con_func;
	pfunc_t pro_func;
	int con_cnt;
	int pro_cnt;
	pthread_t *parr_con;//消费者线程id数组
	pthread_t *parr_pro;
	int flag;
	struct sockaddr_in *cli_info;
}factory_t, *pfactory_t;


void factory_init(pfactory_t pf, int con_cnt, \
		int pro_cnt, pfunc_t con_func, pfunc_t pro_func,\
		int que_capacity);
void factory_start(pfactory_t pf);
void factory_destroy(pfactory_t pf);
void factory_stop(pfactory_t pf);

#endif
