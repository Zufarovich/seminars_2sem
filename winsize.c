#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void print_message(int rub);
void finish(int rub);

int main()
{
	signal(SIGWINCH, &print_message);
	signal(SIGINT, &finish);

	while(1);
}

void print_message(int rub)
{
	struct winsize sz;

	ioctl(0, TIOCGWINSZ, &sz);

	printf("\e[H\e[J\e[%d;%dHSize of window:%d\t%d\n", sz.ws_row/2, sz.ws_col/2, sz.ws_row, sz.ws_col);
}

void finish(int rub)
{
	struct winsize sz;

	ioctl(0, TIOCGWINSZ, &sz);

	printf("\e[H\e[J\e[%d;%dHGood by!\n", sz.ws_row, sz.ws_col/2);

	sleep(3);
	
	exit(0);
}
