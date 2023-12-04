#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdlib.h>

#define BUFFER_LEN 1024
#define FREE_READ 0
#define FREE_WRITE 1

int my_read_write(int fd, size_t size);
void semb_op(struct sembuf* semop, int sem_num, int sem_op);
void my_read(int shm, int id_sem, int fd);
void my_write(int shm, int id_sem);

typedef struct _buffers{
	int finish;
	int w_size[2];
	char buffer[2][BUFFER_LEN];
}buffers_str;

int main(int argc, char* argv[])
{
	int nwrote = 0;

	if (argc == 1)
	{	
		nwrote = my_read_write(0, BUFFER_LEN);	
	
		if (nwrote < 0)
			perror("Error while reading!\n");
	}
	else
	{		
		for	(int i = 1; i < argc; i++)
		{
			int fd = 0;
			if ((fd = open(argv[i], O_RDONLY)) < 0)
				perror("Cannot open file\n");
			else
			{
				nwrote = my_read_write(fd, BUFFER_LEN);				
				
				if (nwrote < 0)
					perror("Error while reading!\n");
					
				close(fd);	
			}
		}
	}	
}


void semb_op(struct sembuf* semop, int sem_num, int sem_op)
{
	semop->sem_num = sem_num;
	semop->sem_op  = sem_op;
	semop->sem_flg = 0;
}

void my_read(int shm_buf, int id_sem, int fd)
{
	int   counter = 0;
	buffers_str* buffer = (buffers_str*)shmat(shm_buf, NULL, 0);
	buffer->finish = 0;
	struct sembuf sops[1];

	semb_op(sops, FREE_READ, -1);//buffer is full
	semop(id_sem, sops, 1);

	while ((buffer->w_size[counter % 2] = read(fd, buffer->buffer[counter % 2], BUFFER_LEN)) > 0)
	{
		semb_op(sops, FREE_WRITE, 1);//waiting for writer
		semop(id_sem, sops, 1);
		counter++;//changing buffer
		semb_op(sops, FREE_READ, -1);//buffer is full
		semop(id_sem, sops, 1);
	}

	buffer->finish = 1;
	semb_op(sops, FREE_WRITE, 1);//increasing for not waiting writer
	semop(id_sem, sops, 1);
	
}

void my_write(int shm_buf, int id_sem)
{
	int   counter = 0;
	buffers_str* buffer = (buffers_str*)shmat(shm_buf, NULL, 0);
	struct sembuf sops[1];
	int size = 1;

	while(size  > 0)
	{
		int sum = 0;
		int nwrite = 0;

		semb_op(sops, FREE_WRITE, -1);//waiting for reader
		semop(id_sem, sops, 1);

		if(buffer->finish && !semctl(id_sem, FREE_WRITE, GETVAL))
			break;

		size = buffer->w_size[counter % 2];

		while (sum < size)
		{
			nwrite = write(1, buffer->buffer[counter % 2] + sum, size - sum);

			sum += nwrite;
		}	

		semb_op(sops, FREE_READ, 1);//free buffer for reading
		semop(id_sem, sops, 1);	
		
		counter++;
	}	
}

int my_read_write(int fd, size_t size)
{
	int nread = 0;
	
	struct sembuf sops[1];
	int id_sem = semget(IPC_PRIVATE, 2, IPC_CREAT|IPC_EXCL|0700);
	sops[0].sem_num = FREE_READ;
	sops[0].sem_op  = 2;
	sops[0].sem_flg = 0;
	semop(id_sem, sops, 1);

	int id_shm = shmget(IPC_PRIVATE, sizeof(buffers_str), IPC_CREAT|IPC_EXCL|0700);
	
	if (!id_shm || !id_sem)
	{
		perror("Error while creating shm");
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	if (!pid)
	{
		my_read(id_shm, id_sem, fd);
		return 0;
	}
	
	pid = fork();
	if (!pid)
	{
		my_write(id_shm, id_sem);
		return 0;
	}
		
	for(int i = 0; i < 2; i++)
		wait(NULL);

	semctl(id_sem, 0, IPC_RMID);
	
	return 1;	
}
