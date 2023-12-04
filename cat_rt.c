#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static pid_t parent_pid;

void reader();
void writer(int sig, siginfo_t* info);
void end_prog(int sig, siginfo_t* info);

int main()
{
	pid_t pid = fork();

	if (pid == 0)
	{
		parent_pid = getppid();
		reader();
	}

	int signal_num;

	struct sigaction action;

	action.sa_handler = writer;
	action.sa_flags = SA_SIGINFO;

	sigset_t mask_act;
	sigfillset(&mask_act);
	sigdelset(&mask_act, SIGRTMIN);
	action.sa_mask = mask_act;

	struct sigaction end;

	end.sa_handler = end_prog;
	end.sa_flags = SA_SIGINFO;

	sigset_t mask_end;
	sigfillset(&mask_end);
	sigdelset(&mask_end, SIGRTMIN + 1);
	sigdelset(&mask_end, SIGINT);
	sigdelset(&mask_end, SIGKILL);
	sigdelset(&mask_end, SIGSTOP);
	end.sa_mask = mask_end;

	sigaction(SIGRTMIN, &action, NULL);
	sigaction(SIGRTMIN + 1, &end, NULL);

	while (1)
	{
		sigwait(&mask_act, &signal_num);
		sigwait(&mask_end, &signal_num);
	}
}

void reader()
{
	char buf[1];
	union sigval mail;
		
	while (read(0, buf, 1) > 0)
	{
		mail.sival_int = buf[0];
		while (sigqueue(parent_pid, SIGRTMIN, mail) < 0)
			continue;
	}
	
	while (sigqueue(parent_pid, SIGRTMIN + 1, mail) < 0)
		continue;
}

void writer(int sig, siginfo_t* info)
{
	union sigval send;
	int buf[1];
	buf[0] = info->si_value.sival_int;

	if (write(1, buf, sizeof(int)) < 0)
		sigqueue(getpid(), SIGRTMIN + 1, send);
}

void end_prog(int sig,siginfo_t* info)
{
	exit(0);
}
