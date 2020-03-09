#include<stdio.h>
#include<string.h>
#include "menu.h"

void input_help()
{
	printf("\nrun <jobname> <burst_time> <priority>:   submit a job named <jobname>,\n\t\t\t\t\t execution time is <burst_time>,\n\t\t\t\t\t priority is <priority>.");
	printf("\nlist: display the job status");
	printf("\nfcfs: change the scheduling policy to FCFS");
	printf("\nsjf: change the scheduling policy to SJF");
	printf("\npriority: change the scheduling policy to priority");
	printf("\ntest <benchmark> <policy> <num_of_jobs> <priority_levels>\n     <min_CPU_time> <max_CPU_time>");
	printf("\nquit: exit AUBatch");
				
}

int splitColon(char time[]){
	char t[30]="";
	char *token = strtok(time, ":");
	int stoi;
	while (token != NULL)
	{
		strcat(t,token);
		token = strtok(NULL, ":");
	}
	sscanf(t, "%d", &stoi);
	return stoi;
}