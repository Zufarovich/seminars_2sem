#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

int BUFFER_LEN = 128;

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		uid_t uid = getuid();
		gid_t gid = getgid();
		gid_t groups[BUFFER_LEN];
		int len = getgroups(BUFFER_LEN, groups);
		struct passwd* name = getpwuid(uid);
		struct group*	gr_name = getgrgid(gid);
		printf("uid=%d(%s) gid=%d(%s) groups=", (int) uid, name->pw_name, (int) gid, gr_name->gr_name);
		for (int i = 0; i < len; i++)
		{	
			gr_name = getgrgid(groups[i]);
			if (i != len - 1)
				printf("%d(%s)," , (int) groups[i], gr_name->gr_name);
			else
				printf("%d(%s)\n", (int) groups[i], gr_name->gr_name);
		}
	}
	else
	{
		struct passwd* info;
		if(!(info = getpwnam(argv[1])))
			perror("You entered invalid name!");
		else
		{
			struct group* gr_name = getgrgid(info->pw_gid);
			printf("uid=%d(%s) gid=%d(%s) groups=", (int) info->pw_uid, info->pw_name, (int) info->pw_gid, gr_name->gr_name);
			int ngroups = BUFFER_LEN;
			gid_t usr_groups[BUFFER_LEN];
			int inform = getgrouplist(argv[1], info->pw_gid, usr_groups,&ngroups);
			while (inform < 0)
			{
				BUFFER_LEN *= 2;
				inform = getgrouplist(argv[1], info->pw_gid, usr_groups,&ngroups);
			}	
			for (int i = 0; i < ngroups; i++)
			{
				gr_name = getgrgid(usr_groups[i]);
				if (i != ngroups - 1)
					printf("%d(%s)," , (int) usr_groups[i], gr_name->gr_name);
				else
					printf("%d(%s)\n", (int) usr_groups[i], gr_name->gr_name);
			}
		}
	}
}
