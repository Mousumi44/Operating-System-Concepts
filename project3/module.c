#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include "module.h"
#define MAX_JOB 10

int count = 0;
int front =0;
int rear = 0;
int total=0;
int wait_time=0;
int turn_time=0;
int cpu_time=0;
   
char policy[30];

struct jobQueue
{  
   int arg1,arg2;
   int burst_time,priority;
   char arr_time[30];
   char name[20];
   char exec_time[30];
   char comp_time[30];
   int num;
  
};
 
struct jobQueue job[MAX_JOB-1];

void perfEvaluate(char arr_time[],char exec_time[],char comp_time[]){
	
	int arr;int exec;int comp;
	arr = splitTime(arr_time);
	exec= splitTime(exec_time);
	comp = splitTime(comp_time);
	wait_time = abs(wait_time+(exec-arr));
	turn_time = abs(turn_time + wait_time+(comp-exec));
	cpu_time = abs(cpu_time+comp);
	
}

void 
run(char command[])
{
	char cmd[50];
	strcpy(cmd,command);
	char *token = strtok(cmd, " ");
	int j=1;
	while (token != NULL)
	{
		if (j==2)
			sscanf(token, "%s", &job[front].name);
		if (j==3)
			sscanf(token, "%d", &job[front].burst_time);
		if (j==4)
			sscanf(token, "%d", &job[front].priority);
		j++;
		token = strtok(NULL, " ");
	}
	if(j!=5){
		printf("\nWrong command");
		
	}
	printf("job %s was submitted.\n", job[front].name);
	time_t T=time(NULL);
	struct tm tm = *localtime(&T);
	sprintf(job[front].arr_time,"%d:%d:%d",tm.tm_hour, tm.tm_min, tm.tm_sec);
	job[front].num = front+1;
			count++;
			front++;
			total++;
	printf("Total number of jobs in the queue: %d\n", total);
	printf("Scheduling Policy: %s\n", policy);

	

}
void fcfs(){
	int min_burst,min_arg1,min_arg2,min_priority,min_num;
	char min_name[20],min_time[30];
	int i,j;
	   for (i = 1; i < count; i++)
	   {   min_num = job[i].num;
		   min_burst = job[i].burst_time;
		   min_arg1 = job[i].arg1;
		   min_arg2 = job[i].arg2;
		   min_priority = job[i].priority;
		   strcpy(min_name , job[i].name);
		   strcpy(min_time ,job[i].arr_time);
		   j = i-1; 
		   while ((j >= 0) && (job[j].num > min_num))
		   {
			   job[j+1].num = job[j].num;  
			   job[j+1].burst_time = job[j].burst_time;
			   job[j+1].arg1 = job[j].arg1;
			   job[j+1].arg2 = job[j].arg2;
			   job[j+1].priority = job[j].priority;
			   strcpy(job[j+1].arr_time ,job[j].arr_time);
			   strcpy(job[j+1].name ,job[j].name);
			   j = j-1;
		   }
		   job[j+1].num = min_num;
		   job[j+1].burst_time = min_burst;
		   job[j+1].arg1 = min_arg1; 
		   job[j+1].arg2 = min_arg2;
		   job[j+1].priority = min_priority;
		   strcpy(job[j+1].arr_time ,min_time);
		   strcpy(job[j+1].name , min_name);
	   }
	   
	
	printf("\nScheduling policy is switched to FCFS. All the %d waiting jobs have been rescheduled.",count);
}
void sjf(){
	int min_burst,min_arg1,min_arg2,min_priority;
	char min_name[20],min_time[30];
	int i,j;
	   for (i = 1; i < count; i++)
	   {
		   min_burst = job[i].burst_time;
		   min_arg1 = job[i].arg1;
		   min_arg2 = job[i].arg2;
		   min_priority = job[i].priority;
		   strcpy(min_name , job[i].name);
		   strcpy(min_time ,job[i].arr_time);
		   j = i-1; 
		   while ((j >= 0) && (job[j].burst_time > min_burst))
		   {
			   job[j+1].burst_time = job[j].burst_time;
			   job[j+1].arg1 = job[j].arg1;
			   job[j+1].arg2 = job[j].arg2;
			   job[j+1].priority = job[j].priority;
			   strcpy(job[j+1].arr_time ,job[j].arr_time);
			   strcpy(job[j+1].name ,job[j].name);
			   j = j-1;
		   }
		   job[j+1].burst_time = min_burst;
		   job[j+1].arg1 = min_arg1; 
		   job[j+1].arg2 = min_arg2;
		   job[j+1].priority = min_priority;
		   strcpy(job[j+1].arr_time ,min_time);
		   strcpy(job[j+1].name , min_name);
	   }
	   
	   printf("\nScheduling policy is switched to SJF. All the %d waiting jobs have been rescheduled.",count);
	  
	}
void priority(){
	int max_priority,max_arg1,max_arg2,max_burst_time;
	char max_name[20],max_time[30];
	int i,j;
	   for (i =1 ; i < count; i++)
	   {
		   max_priority = job[i].priority;
		   max_arg1 = job[i].arg1;
		   max_arg2 = job[i].arg2;
		   max_burst_time = job[i].burst_time;
		   strcpy(max_name,job[i].name);
		   strcpy(max_time ,job[i].arr_time);
		   j = i-1; 
		   while ((j >= 0) && (job[j].priority < max_priority))
		   {
			   job[j+1].priority = job[j].priority;
			   job[j+1].burst_time = job[j].burst_time;
			   job[j+1].arg1 = job[j].arg1;
			   job[j+1].arg2 = job[j].arg2;
			   stpcpy(job[j+1].arr_time,job[j].arr_time);
			   strcpy(job[j+1].name , job[j].name);
			   j = j-1;
		   }
		   job[j+1].priority = max_priority;
		   job[j+1].burst_time = max_burst_time;
		   job[j+1].arg1 = max_arg1; 
		   job[j+1].arg2 = max_arg2;
		   strcpy(job[j+1].arr_time, max_time);
		   stpcpy(job[j+1].name,max_name);
}
	printf("\nScheduling policy is switched to Priority. All the %d waiting jobs have been rescheduled.",count);
		
}

/*Scheduler thread*/
void *scheduling()
{
	int i,j;
	float avg_turntime,avg_waittime,avg_cputime,thro;
	char *command1;
	char command[50];
    size_t input_size = 32;
	command1 = (char *)malloc(input_size * sizeof(char));
		for(i=0;i<MAX_JOB;i++){
		pthread_mutex_lock(&cmd_queue_lock);
        //printf("\nScheduler: count = %d", count);
        while (count == MAX_JOB) {
			/*Waits until the buffer is not full*/
            pthread_cond_wait(&cmd_buf_not_full, &cmd_queue_lock);
        }
		pthread_mutex_unlock(&cmd_queue_lock);
		printf("\n>");		
		getline(&command1,&input_size,stdin);
		
		strncpy(command,command1,strlen(command1)-1);
		if(strcmp(command,"help")==0){
			help();
		}
	
		else if(strstr(command,"run")){				
			pthread_mutex_lock(&cmd_queue_lock); 
			run(command);
			if (front == MAX_JOB)
				front = 0;
			
        pthread_cond_signal(&cmd_buf_not_empty);  
        
        pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(command,"list")==0){
			pthread_mutex_lock(&cmd_queue_lock);
			printf("\nName\tCPU_Time\tPriority\tArrival_time\tProgress");
			for(j=0;j<count;j++){
				printf("\n%s\t%d\t\t%d\t\t%s\tTo be executed",job[j].name,job[j].burst_time,job[j].priority,job[j].arr_time);			
			}	
			pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(command,"fcfs")==0){
			pthread_mutex_lock(&cmd_queue_lock);
			strcpy(policy,"FCFS");
			fcfs();
			pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(command,"sjf")==0){
			pthread_mutex_lock(&cmd_queue_lock);
			strcpy(policy,"SJF");
			sjf();
			pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(command,"priority")==0){
			pthread_mutex_lock(&cmd_queue_lock);
			strcpy(policy,"PRIORITY");
			priority();
			pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(command,"quit")==0){
		avg_turntime = turn_time/total;
		avg_waittime = wait_time/total;
		avg_cputime = cpu_time/total;
		thro = 1.0/avg_turntime;
		printf("\nTotal jobs submitted : %d",total);
		printf("\nAverage turn around time : %f seconds",avg_turntime);
		printf("\nAverage CPU time : %f seconds",avg_cputime);
		printf("\nAverage waiting time : %f seconds",avg_waittime);
		printf("\nThroughput : %f No./second\n",thro);
		exit(0);	
		}
		else{
			
			printf("\n Type help to get the list of commands");
		}
		memset(command,0,sizeof(command));
	}
	
	return NULL;
}
/*Dispatcher thread*/
void *dispatching()
{
    
	int i;
	char arg1[20],arg2[20];
	
	 for (i = 0; i <total+1 ; i++) {
		garbageCompute1();
        pthread_mutex_lock(&cmd_queue_lock);
        
        //printf("\nDispatcher: count = %d", count);
		
        while (count == 0) {
			/*Waits until the buffer has atleast one job*/
            pthread_cond_wait(&cmd_buf_not_empty, &cmd_queue_lock);
			
        }
        count--;
        
        sprintf(arg1,"%d",job[rear].arg1);
		sprintf(arg2,"%d",job[rear].arg2);
		time_t T1=time(NULL);
		struct tm tm1 = *localtime(&T1);
		sprintf(job[rear].exec_time,"%d:%d:%d",tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
		pid_t forked = fork();
		if (forked==0){
			
			execv("./process",(char*[]){"./process",arg1,arg2,NULL});

		}
		time_t T3=time(NULL);
		garbageCompute2();
		struct tm tm2 = *localtime(&T3);
		sprintf(job[rear].comp_time,"%d:%d:%d",tm2.tm_hour, tm2.tm_min, tm2.tm_sec);
		
		
		perfEvaluate(job[rear].arr_time,job[rear].exec_time,job[rear].comp_time);		
		
        rear++;
        if (rear == MAX_JOB){
			rear = 0; 
			}

      pthread_cond_signal(&cmd_buf_not_full);
       
      pthread_mutex_unlock(&cmd_queue_lock); 
        
       
	 }
      return NULL;   
}

void garbageCompute1()
{
	for (int i=0;i< 1000000; i++)
	{
		for(int j=0;j<10000;j++)
		{

		}
	}
}

void garbageCompute2()
{
	for (int i=0;i< 10000; i++)
	{
		for(int j=0;j<100;j++)
		{

		}
	}
}

