#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static int byte;
static int bit_in_byte = 7;
static pid_t parent_pid;

static char byte_s;
static int bit_in_byte_s = -1;
static pid_t child_pid;

ssize_t my_write(int fd, const char* buff, size_t size_str);
void bit_receive(int num_sign);
void bit_send(int num_sign);
void end_prog_success();
void end_prog_fail();

void end_prog_success() {
    exit(0);
}

void end_prog_fail(){
	exit(EXIT_FAILURE);
}

void bit_receive(int num_sign) {
    int bit = 0;

    if (num_sign == SIGUSR1)			//setting bit that have been read
        bit = 0;
    
    if(num_sign == SIGUSR2)				//setting bit that have been read
        bit = 1;
    
    bit = bit << bit_in_byte;    
    byte += bit;
    bit_in_byte--;    

    if (bit_in_byte == -1)				//writing byte in case it fully filled
	{
        char buf[1];
        buf[0] = byte;
        if (write(1, buf, 1) < 0)
		{
            kill(parent_pid, SIGINT);                
			exit(EXIT_FAILURE);
        }
        byte = 0;
        bit_in_byte = 7;
    }

    
    kill(parent_pid, SIGUSR1);
}

void bit_send(int num_sign) 
{
    if (bit_in_byte_s == -1)			//initializing and reading byte
	{
        byte_s = 0;
        char buf[1];
		int read_return = -1;
    
        if ((read_return = read(0, buf, 1)) == 0)		//signal that everything is read or smth went wrong
		{
            kill(child_pid, 0);
			exit(0);
        }
		else if (read_return < 0)
		{
			kill(child_pid, SIGINT);
			exit(EXIT_FAILURE);
		}
		
        byte_s = buf[0];
        bit_in_byte_s = 7;
    }

    bit_in_byte_s -= 1;

    if (byte_s & (1 << (bit_in_byte_s + 1)))     //reading and sending bit
        kill(child_pid, SIGUSR2);
    else 
        kill(child_pid, SIGUSR1);
}

int main() 
{
    pid_t pid = fork();					//creating writer process

    if (pid == 0) 
	{
        signal(SIGUSR1, bit_receive);	//processing received byte
        signal(SIGUSR2, bit_receive);	//processing received byte
        signal(0,  end_prog_success);	//ending programm in case reading is ended or smth happend
		signal(SIGINT, end_prog_fail);
        parent_pid = getppid();

        sleep(1);

        kill(parent_pid, SIGUSR1);		//starting process of reading and sending bytes

        while (1) 
            pause();					//waiting for signal
    }

    signal(SIGUSR1, bit_send);			//reading and sending bytes
    signal(SIGUSR2, bit_send);			//reading and sending bytes
    signal(0, end_prog_success);		//starting process of reading and sending bytes
	signal(SIGINT, end_prog_fail);
    child_pid = pid;

    while (1) 
        pause();						//waiting for signal

}
