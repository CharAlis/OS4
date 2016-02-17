#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

int ft();
void convertmode(int mode, char *perms);

int main(int argc, char const *argv[])
{
	ft();
	exit(EXIT_SUCCESS);
}

int ft()
{
	struct stat stats;
	struct dirent *procentry, *entry;
	struct passwd  *pwd;
	struct group   *grp;
	FILE *file;
	DIR *proc, *process;
	long my_pid, my_ppid, my_uid, my_gid, target_pid, target_ppid;
	char buffer[512], tmp[512], temp[512], permissions[11];
	int count = 0, first, procnum;

	my_pid = (long) getpid();
	my_ppid = (long) getppid();

	// Get uid, gid of our process
	strcpy(buffer, "/proc/");
	sprintf(temp, "%ld", my_pid);
	strcat(buffer, temp);

	if (stat(buffer, &stats) == -1)
	{
		perror("stat");
		exit(EXIT_FAILURE);
	}

	my_uid = stats.st_uid;
	my_gid = stats.st_gid;

	// Start finding processes
	proc = opendir("/proc");
	while ((procentry = readdir(proc)) != NULL)
	{
		first = 1;
		switch (procentry->d_type)
		{
			case DT_DIR:
				if (sscanf(procentry->d_name, "%d", &procnum) < 1) break;	// Allow only entry in process folders
				if (strcmp(procentry->d_name, ".") && strcmp(procentry->d_name, ".."))
				{
					strcpy(tmp, "/proc/");
					strcat(tmp, procentry->d_name);

					// Getting stats to verify process ownership of user
					if (stat(tmp, &stats) == -1)
					{
						perror("stat");
						exit(EXIT_FAILURE);
					}

					// Check if process originated from the same tty, skip self
					strcpy(buffer, tmp);
					strcat(buffer, "/stat");
					file = fopen(buffer, "r");
					fscanf(file, "%ld %*s %*c %ld", &target_pid, &target_ppid);
					fclose(file);
					if ((my_ppid != target_ppid) || (my_pid == target_pid)) continue;

					// Make sure user id and group id match
					if (stats.st_uid != my_uid || stats.st_gid != my_gid) break;

					// Finding open files for process
					strcat(tmp, "/fd");
					// Checking all contents of fd folder
					if ((process = opendir(tmp)) != NULL)
					{
						while ((entry = readdir(process)) != NULL)	// access only content allowed
						{
							if ((entry->d_type == DT_LNK) && (atoi(entry->d_name) > 2))	// if it is a link to an open file, other than standard iput/output/error streams
							{
								strncpy(temp, "", sizeof(temp));
								strcpy(buffer, tmp);
								strcat(buffer, "/");
								strcat(buffer, entry->d_name);
								readlink(buffer, temp, sizeof(temp));	// path and file name

								if (stat(buffer, &stats) == -1)
								{
									perror("stat");
									exit(EXIT_FAILURE);
								}
								
								// Each time we print for a new process, we print out it's PID
								if (first)
								{
									first = 0;
									if (count) printf("\n");
									printf("PID %ld\n", target_pid);
								}

								// Printing out results
								convertmode(stats.st_mode, permissions);
								if (((pwd = getpwuid(stats.st_uid)) != NULL) && ((grp = getgrgid(stats.st_gid)) != NULL))
									printf("%10.10s %2d %-8.8s %-6.6s %4ld %s\n", permissions, (int)stats.st_nlink, pwd->pw_name, grp->gr_name, (long)stats.st_size, temp);
								else
									printf("%10.10s %2d %-8d %-6d %4ld %s\n", permissions, (int)stats.st_nlink, stats.st_uid, stats.st_gid, (long)stats.st_size, temp);
								fflush(stdout);
								count++;
							}
						}
					}
				}
				break;
		}
	}
	if (count)
		printf("-------------------\nTotal open files: %d\n", count);
	exit(EXIT_SUCCESS);
}

void convertmode(int mode, char *perms)
{
	// leading char
	if (S_ISDIR(mode))
		perms[0] = 'd';
	/*else if (S_ISCHR(mode))
		perms[0] = 'c';
	else if (S_ISBLK(mode))
		perms[0] = 'b';
	else if (S_ISFIFO(mode))
		perms[0] = 'p';
	else if (S_ISLNK(mode))
		perms[0] = 'l';
	else if (S_ISSOCK(mode))
		perms[0] = 's';*/
	else
		perms[0] = '-';

	// following chars
	perms[1] = (mode & S_IRUSR) ? 'r' : '-';
	perms[2] = (mode & S_IWUSR) ? 'w' : '-';
	perms[3] = (mode & S_IXUSR) ? 'x' : '-';
	perms[4] = (mode & S_IRGRP) ? 'r' : '-';
	perms[5] = (mode & S_IWGRP) ? 'w' : '-';
	perms[6] = (mode & S_IXGRP) ? 'x' : '-';
	perms[7] = (mode & S_IROTH) ? 'r' : '-';
	perms[8] = (mode & S_IWOTH) ? 'w' : '-';
	perms[9] = (mode & S_IXOTH) ? 'x' : '-';
	perms[10] = '\0';
}