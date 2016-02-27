#include "factory.h"

void factory_init(pfactory_t pf, int con_cnt, int pro_cnt,\
		pfunc_t con_func, pfunc_t pro_func, int que_capacity)
{
	int ret;
	pf->pque = (pque_t)malloc(sizeof(que_t));
	ft_que_init(pf->pque, que_capacity);
	pf->con_func = con_func;
	pf->pro_func = pro_func;
	pf->con_cnt = con_cnt;
	pf->pro_cnt = pro_cnt;
	pf->parr_con = (pthread_t*)calloc(sizeof(pthread_t), con_cnt);
	pf->parr_pro = (pthread_t*)calloc(sizeof(pthread_t), pro_cnt);
	ret = pthread_cond_init(&pf->cond, NULL);
	pf->cli_info = malloc(sizeof(struct sockaddr_in));
	if(ret == -1)
		err_quit("pthread_cond_init error");
	pf->flag = 0;
}

void factory_start(pfactory_t pf)
{
	if(pf->flag)
		return;
	int cnt;
	for(cnt = 0; cnt < pf->con_cnt; cnt++)
		if(pthread_create(pf->parr_con+cnt, NULL, pf->con_func, pf) == -1)
			err_quit("pthread_create consumer error");
	for(cnt = 0; cnt < pf->pro_cnt; cnt++)
		if(pthread_create(pf->parr_pro+cnt, NULL, pf->pro_func, pf) == -1)
			err_quit("pthread_create producer error");
	pf->flag = 1;
}
