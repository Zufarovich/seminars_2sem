#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_LEN 32768

struct timespec start, end;

int my_read_wc(int fdin, int* answer);

int main(int argc, char* argv[])
{
	int pipefd[2];
	int answer[3] = {0, 0, 0};

	pipe(pipefd);
	pid_t pid = fork();

	if(pid == 0)
	{
		dup2(pipefd[1], 1);
		close(pipefd[1]);
		execvp(argv[1], &argv[1]);
		perror("Exec failed");
		return 0;
	}

	close(pipefd[1]);

	my_read_wc(pipefd[0], answer);
	wait(NULL);

	printf("\t%d\t%d\t%d\n", answer[0], answer[1], answer[2]);
}

int my_read_wc(int fdin, int* answer)
{
	int nread = 0;
	char buffer[BUFFER_LEN] = {};
	char save = '\0';
	int first_char = 1;

	while((nread = read(fdin, buffer, BUFFER_LEN)) > 0)
	{
		if (first_char)
			if(!isspace(buffer[0]))
				answer[1]++;

		first_char = 0;		

		for(int i = 0; i < nread; i++)
		{
			if(buffer[i] == '\n')
				answer[0]++;

			if(isspace(save) && !isspace(buffer[i]))
				answer[1]++;

			save = buffer[i];
		}

		answer[2] += nread;
	}

	return 1;	
}
