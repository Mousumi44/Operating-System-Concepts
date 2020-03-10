#ifndef MODULE_H
#define MODULE_H

/*Lock and condition variables*/
pthread_mutex_t cmd_queue_lock;  
pthread_cond_t cmd_buf_not_full; 
pthread_cond_t cmd_buf_not_empty;

void *scheduling();
void *dispatching();
void run(char command[]);
void fcfs();
void sjf();
void priority();
void perfEvaluate();
void garbageCompute1();
void garbageCompute2();

#endif //MODULE_H