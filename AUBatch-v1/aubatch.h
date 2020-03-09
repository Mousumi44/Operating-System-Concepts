#ifndef AUBATCH_H
#define AUBATCH_H

/*Lock and condition variables*/
pthread_mutex_t cmd_queue_lock;  
pthread_cond_t cmd_buf_not_full; 
pthread_cond_t cmd_buf_not_empty;

void *scheduling();
void *dispatching();
void input_run(char input_cmd[]);
void fcfs();
void sjf();
void priority();
void perfEvaluate();

#endif //AUBATCH_H