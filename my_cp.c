#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define HELP	1
#define VERBOSE 2
#define BUFFER_LEN 1024

struct option long_options[] = {
	{"force",		 no_argument, 0, 'f' },
	{"interactive",  no_argument, 0, 'i' },
	{"verbose",		 no_argument, 0, 'v' },
	{"help",		 no_argument, 0, 'h' },
	{"preserve",	 no_argument, 0, 'p' }
};

void    write_into_dir(char* argv[], int argc, int* files, int non_keys_args, int flag, int key_print, int copy_mode);
ssize_t my_write(char* buffer, size_t size, int dest);
int     my_read_write(int src, size_t size, int dest);
int     open_file(char* path, int flag, int mode);
char*   concat_addr(char* path, char* file);
mode_t     get_stat(char* path, int mode_check);
void    close_files(int* files, int amount);

int main(int argc, char* argv[])
{
	int  opt          = '\0';
	int  flag         = O_CREAT | O_WRONLY;
	int  option_index = 0;
	int  key_print    = 0;
	int  copy_mode    = 0;

	while((opt = getopt_long(argc, argv, "fivp", long_options, &option_index)) != -1)
	{
		switch(opt){
		case 'f':
			flag = flag | O_TRUNC;
			break;
		case 'i':
		    flag = flag | O_EXCL;
			break;
		case 'v':
			key_print = VERBOSE;
			break;
		case 'h':
			key_print = HELP;
			break;
		case 'p':
			copy_mode = 1;
		  break;
		default:
			printf("with command \"cp --help\" you can get additional information.\n");
			break;
		}
	}	
	
	if (key_print == HELP)
	{
		printf("Usage: cp [parameter]... SOURCE DESTINATION\n   or: cp [parameter]... SOURCE... DIRECTORY\n");
		printf("Copies SOURCE into DESTINATION or several SOURCES into DIRECTORY\n");
	}
	else
	{
		int non_keys_args = argc - optind;

		if (non_keys_args < 2)
			printf("my_cp: after \'%s\' skipped the setting operand of target file\nwith command \"cp --help\" you can get additional information.\n", argv[optind]);	
		else
		{
			if (non_keys_args >= 2)
			{
				int*   files    = (int*)malloc(sizeof(int) * non_keys_args);
				mode_t dst_mode = 00700;
				int    src_mode = -1;
	
				for (int i = optind; i < argc - 1; i++)
				{
					if ((src_mode = get_stat(argv[i], S_IFREG)) > 0)
						files[i - optind] = open_file(argv[i], O_RDONLY, 00700);
					else if (src_mode == 0)
						printf("%s is not a regular file skipping\n", argv[i]);
					else
						perror(argv[i]);
				}
				
				if (non_keys_args != 2)
				{
					if ((src_mode = get_stat(argv[argc - 1], S_IFDIR)) == 0)
						printf("%s is not a directory\n", argv[argc - 1]);
					else  if(src_mode > 0)
						write_into_dir(argv, argc, files, non_keys_args, flag, key_print, copy_mode);			
					else
						perror(argv[argc - 1]);
				}	
				else
				{
					if (get_stat(argv[argc - 1], S_IFDIR))
						write_into_dir(argv, argc, files, non_keys_args, flag, key_print, copy_mode);	
					else
					{
						if (copy_mode)
							dst_mode = get_stat(argv[optind], S_IFREG);

						if ((files[non_keys_args - 1] = open_file(argv[argc - 1], flag, dst_mode)) > 0)
							my_read_write(files[optind - 1], BUFFER_LEN, files[non_keys_args - 1]);
					}
				}

				close_files(files, non_keys_args - 1);	

				free(files);
			}		
		}
	}
}

void write_into_dir(char* argv[], int argc, int* files, int non_keys_args, int flag, int key_print, int copy_mode)
{	
	for (int  i = 0; i < non_keys_args - 1; i++)
	{
		if (files[i] > 0)
		{
			int    fd_copy_file  = -1;
			char*  concated_path = concat_addr(argv[argc - 1], argv[i + optind]);
			mode_t dst_mode      = 00700;
			
			if (copy_mode)
				dst_mode = get_stat(argv[i + optind], S_IFREG);

			if ((fd_copy_file = open_file(concated_path, flag, dst_mode)) < 0)
				free(concated_path);
			else if (fd_copy_file != 0)
			{
				my_read_write(files[i], BUFFER_LEN, fd_copy_file);

				if (key_print == VERBOSE)
					printf("\'%s\' -> '\%s\'\n", argv[optind + i], concated_path);

				close(fd_copy_file);
			}

			free(concated_path);
		}	
	}
}

char* concat_addr(char* path, char* file)
{
	int name_index = 0;
	int i = 0;

	while (file[i] != '\0')
	{
		if (file[i] == '/')
			name_index = i + 1;
		
		i++;
	}

	char* concated_str = (char*) calloc(strlen(path) + strlen(file) - name_index + 1, sizeof(char));
	
	if (concated_str == NULL)
		exit(EXIT_FAILURE);
	else
	{
		strcat(concated_str, path);
		strcat(concated_str, &file[name_index]);

		return concated_str;
	}
}

mode_t get_stat(char* path, int mode_check)
{
	struct stat sb;

	if (stat(path, &sb) < 0)
		return -1;
	else
	{
		if ((sb.st_mode & S_IFMT) == mode_check)
			return sb.st_mode;
		else
			return 0;
	}
}

int open_file(char* path, int flag, int mode)
{
	int fd = -1;

	if ((fd = open(path, flag, mode)) < 0)
	{
		if (errno != EEXIST){
			perror("Cannot open file");
			return -1;	
		}
		else
		{
			printf("Do you want to rewrite existing file %s?:", path);

			char ch = getchar();
			while (getchar() != '\n')
				continue;

			if (ch == 'y' || ch == 'Y'){
				return open(path, flag & (~O_EXCL), mode);
			}
			else{
				return 0;
			}
		}	
	}

	return fd;
}

void close_files(int* files, int amount)
{
	for (int i = 0; i < amount; i++)
	{
		if (files[i] > 0)
			close(files[i]);
	}
}

ssize_t my_write(char* buffer, size_t size, int dest)
{	
	int sum = 0;
	int nwrite = 0;
	while (sum < size)
	{
		nwrite = write(dest, buffer + sum, size - sum);
		
		if (nwrite < 0)
			return -1;
		
		sum += nwrite;
	}
	
	return sum;
}

int my_read_write(int src, size_t size, int dest)
{
	int  nread = 0;
	char buffer[BUFFER_LEN] = {};

	while((nread = read(src, buffer, BUFFER_LEN)) > 0)
	{
		if(my_write(buffer, nread, dest) < 0)
		{	
			perror("Error while writing!\n");
			
			return -1;	
		}
	}
	
	return 1;	
}
