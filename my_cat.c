#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_LEN 1024

int my_read_write(int fd, size_t size);
ssize_t my_write(char* buffer, size_t size);

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

ssize_t my_write(char* buffer, size_t size)
{	
	int sum = 0;
	int nwrite = 0;
	while (sum < size)
	{
		nwrite = write(1, buffer + sum, size - sum);
		
		if (nwrite < 0)
			return -1;
		
		sum += nwrite;
	}
	
	return sum;
}

int my_read_write (int fd, size_t size)
{
	int nread = 0;
	char buffer[BUFFER_LEN] = {};

	while((nread = read(fd, buffer, BUFFER_LEN)) > 0)
	{
		if(my_write(buffer, nread) < 0)
		{	
			perror("Error while writing!\n");
			
			return -1;	
		}
	}
	
	return 1;	
}	
