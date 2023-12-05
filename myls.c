#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

int print_info(char* path);

#define BUFFER_LEN 2048

int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; i++)
		print_info(argv[i]);
}

int print_info(char* path)
{
	DIR* dp = opendir(path);
	//perror("opendir");
	
	if(dp == 0)
		return;

	struct dirent* dif = readdir(dp);
	
	while (dif != NULL)
	{
		if ((dif->d_name[0] == '.' && dif->d_name[1] == '.' && dif->d_name[2] == '\0') || (dif->d_name[0] == '.' && dif->d_name[1] == '\0') )
			dif = readdir(dp);

		if(dif->d_type == DT_DIR)
		{
			char concated_str[BUFFER_LEN] = {};
			sprintf(concated_str, "%s/%s", path, dif->d_name);
			print_info(concated_str);
		}		

		printf("%s/%s\n", path, dif->d_name);

		dif = readdir(dp);
	}

	closedir(dp);
}
