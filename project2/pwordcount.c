#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 50
#define READ_END        0
#define WRITE_END       1

int main(int argc, char*argv[])
{
	/*Error Checking: No file name*/
	if(argc!=2)
	{
		printf("Please enter a file name.\nUsage: ./pwordcount <file_name>\n");
	}
       	pid_t pid;

	/*Declaring Pipe*/
        int fd1[2];
        int fd2[2];

        /* create the pipe */
        if (pipe(fd1) == -1 || pipe(fd2) == -1) {
                fprintf(stderr,"Pipe failed");
                return 1;
        }
	/* now fork a child process */
        pid = fork();

        if (pid < 0) {
                fprintf(stderr, "Fork failed");
                return 1;
        }
	if (pid > 0) {  /* parent process or process1 */

		FILE *file = fopen(argv[1], "r");
		/*Error Checking: can't open file */
		if(file==0)
		{
			printf("Could not open file: %s.\n",argv[1]);
			return 1;
		}
		else
		{

		/*pwordcount continues...*/
		printf("Process 1 is reading file '%s' now ...\n",argv[1]);
                /* close the unused end of the pipe1 */
                close(fd1[READ_END]);

		/*Store Data to a Buffer*/
		int c, i=0;
		char p1Buffer[BUFFER_SIZE];
		do{
			c=fgetc(file);
			if(feof(file))
			{
				break;
			}
			p1Buffer[i]=c;
			i++;
		}while(1);


                /* write to the pipe1 */
		printf("Process 1 starts sending data to Process 2 ...\n");
		write(fd1[WRITE_END], p1Buffer, sizeof(p1Buffer)+1);

	       	/* close the write end of the pipe1 */
		close(fd1[WRITE_END]);

		/* close the unused end of pipe2*/
		close(fd2[WRITE_END]);

		/*read from pipe2*/
		int wordCount[2];
		read(fd2[READ_END], wordCount, sizeof(wordCount)+1);
		printf("Process 1: The total number of words is %d\n",wordCount[0]);
		/*close the read end of pipe2*/
		close(fd2[READ_END]);
		}
        }
	else { /* child process or process2 */
                /* close the unused end of the pipe1 */
		close(fd1[WRITE_END]);

		char p2Buffer[BUFFER_SIZE];

                /* read from the pipe1 */
		read(fd1[READ_END],p2Buffer, sizeof(p2Buffer)+1);
		printf("Process 2 finishes receiving data from Process 1 ...\n");

                /* close the read end of the pipe1 */
		close(fd1[READ_END]);

		printf("Process 2 is counting words now ...\n");

		int blank=1, i=0;
		do{
			if(p2Buffer[i] == ' '){
				blank++;
			}
			i++;

		}while(i<=BUFFER_SIZE);

		int wordCount[2];
		wordCount[0] = blank;

		printf("Process 2 is sending the result back to Process 1 ...\n ");
                /*Close the unused end of pipe2*/
		close(fd2[READ_END]);

		/*Write result to the pipe2*/
		write(fd2[WRITE_END], wordCount, 2);

		/*Close the write end of pipe2*/
		close(fd2[WRITE_END]);
		exit(0);

        }

        return 0;
}

