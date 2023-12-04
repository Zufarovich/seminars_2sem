#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	if (argc == 1)
		printf("\n");
	else
	{
		if (!strcmp(argv[1], "-n"))
		{
			for (int i = 2; i < argc; i++)
			{	
				if (i != argc - 1)
					printf("%s ", argv[i]);
				else
					printf("%s", argv[i]);
			}
		}
		else if (!strcmp(argv[1], "--help") && argc == 2)
		{
			printf("It's help\n");
		}
		else
		{
			for (int i = 1; i < argc; i++)
				printf("%s ", argv[i]);

			printf("\n");
		}
	}
}
