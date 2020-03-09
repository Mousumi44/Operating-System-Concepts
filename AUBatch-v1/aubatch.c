#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include "module.h"

pthread_t tid[2];

int main()
{
	int err1,err2,j;
	printf("Welcome to Mousumi's batch job scheduler version 1.0\nType 'help' to find more about AUBatch commands.");
	err1=pthread_create(&(tid[1]),NULL,scheduling,NULL);
	if(err1 != 0)
		printf("\n Cannot create thread : [%s]",strerror(err1));
	err2=pthread_create(&(tid[2]),NULL,dispatching,NULL);
	if(err2 != 0)
		printf("\n Cannot create thread : [%s]",strerror(err2));
	pthread_mutex_init(&cmd_queue_lock, NULL);
	pthread_cond_init(&cmd_buf_not_full, NULL);
	pthread_cond_init(&cmd_buf_not_empty, NULL);
	pthread_join(tid[1],NULL);
	pthread_join(tid[2],NULL);
	return 0;
}
