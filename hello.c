#include <stdio.h>
#include <unistd.h>

int main()
{
	printf("hello ");
	write(1, "world ", 6);
	printf("good bye ");
	write(1, "see ", 4);
}
