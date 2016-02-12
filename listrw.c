#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

int listrw(const char *type, int flag, const char *strtime, const char *dir);
int findpos(const char *string, char c);
void check(const char *dir, time_t curtime, long timelimit, int flag, struct dirent *entry, struct stat *pstats);

int main(int argc, char const *argv[])
{
	int i, flagLR = 0, flagLW = 0, flagType = 0, flagTime = 0, flagDir = 0;
	char strtime[128], dir[256], type[32];

	for (i = 1; i < argc; i++)
	{
		if (!strcmp("-type", argv[i]))
		{
			if (argc > i + 1)
			{
				flagType = 1;
				strcpy(type, argv[i + 1]);
			}
		}
		else if (!strcmp("-lr", argv[i]))
		{
			flagLR = 1;
		}
		else if (!strcmp("-lw", argv[i]))
		{
			flagLW = 1;
		}
		else if (argv[i][0] == '-')
		{
			flagTime = 1;
			strcpy(strtime, argv[i]);
		} else {
			flagDir = 1;
			strcpy(dir, argv[i]);
		}
	}

	if (!(flagLR || flagLW) || !flagTime || !flagDir)
	{
		fprintf(stderr, "Wrong arguments for calling program with -lr/-lw mode\n");
		fprintf(stderr, "use : ~$ ./listrw -lr -2d -type fd <directory>\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		if (flagType)
		{
			if (flagLR) listrw(type, 0, strtime, dir);	// access
			else listrw(type, 1, strtime, dir);	// modification
		}
		else
		{
			if (flagLR) listrw(NULL, 0, strtime, dir);	// access
			else listrw(NULL, 1, strtime, dir);	// modification
		}
	}

	return 0;
}

int listrw(const char *type, int flag, const char *strtime, const char *dir)
{
	struct dirent *entry;
	struct stat stats;
	DIR *directory;
	char buffer[1024];
	int i, flagLR = 0, flagLW = 0, flagType = 0, flagTime = 0, flagDir = 0, limdays = 0, limhours = 0, limmins = 0, limsecs = 0, typeflags[7];
	time_t curtime = time(NULL);
	long timediff = 0, days = 0, hours = 0, minutes = 0, seconds = 0, timelimit;

	// Extracting time
	char temp[16];
	int init = 0, pos = 0, t = 0;
	i = 0;
	while (strtime[i] != '\0')
	{
		if (strtime[i] == '-') { i++; continue; }
		if (strtime[i] >= '0' || strtime[i] <= '9') temp[t++] = strtime[i];
		if (strtime[i] == 'd' || strtime[i] == 'h' || strtime[i] == 'm' || strtime[i] == 's')
		{
			temp[t] = '\0';
			t = 0;
			switch (strtime[i])
			{
				case 'd':
					limdays = atoi(temp);
					break;
				case 'h':
					limhours = atoi(temp);
					break;
				case 'm':
					limmins = atoi(temp);
					break;
				case 's':
					limsecs = atoi(temp);
					break;
			}
		}
		i++;
	}

	timelimit = limdays * (24 * 3600) + limhours * 3600 + limmins * 60 + limsecs;

	// Extracting types
	if (type == NULL) {
		flagType = 0;
	}
	else
	{
		flagType = 1;
		for (i = 0; i < 7; i++)
			typeflags[i] = 0;

		i = 0;
		while (type[i] != '\0')
		{
			switch (type[i++])
			{
				case 'f':
					typeflags[0]++;
					break;
				case 'd':
					typeflags[1]++;
					break;
				case 'l':
					typeflags[2]++;
					break;
				case 'p':
					typeflags[3]++;
					break;
				case 'c':
					typeflags[4]++;
					break;
				case 'b':
					typeflags[5]++;
					break;
				case 's':
					typeflags[6]++;
					break;
			}
		}
	}

	// Executing command
	directory = opendir(dir);
	if (directory == NULL) { perror("opendir()"); exit(EXIT_FAILURE); }

	while ((entry = readdir(directory)) != NULL)
	{
		switch (entry->d_type)
		{
			// regular files
			case DT_REG:
				if (flagType == 0 || typeflags[0])
				{
					check(dir, curtime, timelimit, flag, entry, &stats);
				}
				break;
			// directories
			case DT_DIR:
				if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
				{
					strcpy(buffer, dir);
					strcat(buffer, "/");
					strcat(buffer, entry->d_name);

					if (stat(buffer, &stats) < 0)
					{
						perror("stat()");
						exit(EXIT_FAILURE);
					}
					// printf("Getting stats of directory '%s'\n", buffer);

					if (flag)
						timediff = difftime(curtime, stats.st_mtime);
					else
						timediff = difftime(curtime, stats.st_atime);

					if (timediff <= timelimit)
					{
						if (flagType == 0 || typeflags[1]) printf("%s\n", buffer);
						listrw(type, flag, strtime, buffer);
					}
				}
				break;
			// symbolic links
			case DT_LNK:
				if (flagType == 0 || typeflags[2])
				{
					check(dir, curtime, timelimit, flag, entry, &stats);
				}
				break;
			// pipes
			case DT_FIFO:
				if (flagType == 0 || typeflags[3])
				{
					check(dir, curtime, timelimit, flag, entry, &stats);
				}
				break;
			// char devices
			case DT_CHR:
				if (flagType == 0 || typeflags[4])
				{
					check(dir, curtime, timelimit, flag, entry, &stats);
				}
				break;
			// block devices
			case DT_BLK:
				if (flagType == 0 || typeflags[5])
				{
					check(dir, curtime, timelimit, flag, entry, &stats);
				}
				break;
			// sockets
			case DT_SOCK:
				if (flagType == 0 || typeflags[6])
				{
					check(dir, curtime, timelimit, flag, entry, &stats);
				}
				break;
		}
	}
	closedir(directory);

	return 0;
}

int findpos(const char *string, char c)
{
	int pos = 0;
	while (string[pos] != '\0')
	{
		if (string[pos] == c) return pos;
		pos++;
	}
	return -1;
}

void check(const char *dir, time_t curtime, long timelimit, int flag, struct dirent *entry, struct stat *pstats)
{
	char buffer[512];
	int timediff;
	struct stat stats = *pstats;

	strcpy(buffer, dir);
	strcat(buffer, "/");
	strcat(buffer, entry->d_name);

	if (stat(buffer, &stats) < 0)
	{
		perror("stat()");
		exit(EXIT_FAILURE);
	}

	if (flag)
		timediff = difftime(curtime, stats.st_mtime);
	else
		timediff = difftime(curtime, stats.st_atime);

	if (timediff <= timelimit) printf("%s\n", buffer);
}