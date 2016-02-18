#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include "functions.h"

/*---------CNT-----------*/
int cnt(const char *dirName, int flg)
{
	struct dirent *entry;
	DIR *directory;
	directory = opendir(dirName);
	char tmp[512];
	static int counter = 0;
	static int n_rf = 0; 
	static int n_dirs = 0;
	static int n_sl = 0;
	static int n_np = 0;
	static int n_cd = 0;
	static int n_s = 0;
	static int n_bdf = 0;
	int sum = 0;
	while ((entry = readdir(directory)) != NULL)
	{
		switch (entry->d_type)
		{
			case DT_REG:
						n_rf++;
						break;
			case DT_DIR:
						n_dirs++;
						if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
						{
							strcpy(tmp, dirName);
							strcat(tmp, "/");
							counter++;
							cnt(strcat(tmp,entry->d_name), counter);
						}
						break;
			case DT_LNK:
						n_sl++;
						break;
			case DT_FIFO:
						n_np++;
						break;
			case DT_CHR:
						n_cd++;
						break;
			case DT_SOCK:
						n_s++;
						break;
			case DT_BLK:
						n_bdf++;
						break;
		}

	}
	closedir(directory);
	if (flg == 0)
	{
		n_dirs -= (counter + 1) * 2; //In order not to count the "." and ".." directories more than once
		sum = n_rf + n_dirs + n_sl + n_np + n_cd + n_s + n_bdf;
		printf("Number of regular files:\t%3d\t%3d%%\n", n_rf, (n_rf * 100)/sum);
		printf("Number of directories:\t\t%3d\t%3d%%\n", n_dirs, (n_dirs * 100)/sum);
		printf("Number of symbolic links:\t%3d\t%3d%%\n", n_sl, (n_sl * 100)/sum);
		printf("Number of named pipes:\t\t%3d\t%3d%%\n", n_np, (n_np * 100)/sum);
		printf("Number of char devices:\t\t%3d\t%3d%%\n", n_cd, (n_cd * 100)/sum);
		printf("Number of sockets:\t\t%3d\t%3d%%\n", n_s, (n_s * 100)/sum);
		printf("Number of block device files:\t%3d\t%3d%%\n", n_bdf, (n_bdf * 100)/sum);
	}
	return 0;
}
/*---------CNT-----------*/

/*---------LISTRW---------*/
int listrw(const char *type, int flag, const char *strtime, const char *dir)
{
	struct dirent *entry;
	struct stat stats;
	DIR *directory;
	char buffer[1024];
	int i, flagLR = 0, flagLW = 0, flagType = 0, flagTime = 0, flagDir = 0, limdays = 0, limhours = 0, limmins = 0, limsecs = 0, typeflags[7];
	time_t curtime = time(NULL);
	long timediff = 0, days = 0, hours = 0, minutes = 0, seconds = 0, timelimit;

	// Extracting time from arguments
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
	// Converting time limit to seconds
	timelimit = limdays * (24 * 3600) + limhours * 3600 + limmins * 60 + limsecs;

	// Extracting types (if types argument exists)
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

// Finds the first occuring position of character c inside the string
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

// Checks to see if the last access or modification time is inside the time limit, and if it is, we print out the directory/file/link etc.
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
/*---------LISTRW---------*/

/*---------PS-----------*/
int ps()
{
	struct stat buf;
	struct dirent *entry;
	DIR *directory;
	int pid, i;
	int ppid = getppid();
	char path[128] = "/proc/";
	char line[128];
	char tmp[128];
	long int vsize = 0;
	char cmdline[128];
	char fields[24][128];
	long total_size;
	float total_cpu;
	
	directory = opendir("/proc");
	printf("PID\tPPID\tCPU\tSTATE\tSTART\tVSZ\tRSS\tPRIORITY\tCMDLINE\n");
	while ((entry = readdir(directory)) != NULL)
	{
		if (sscanf(entry->d_name, "%d", &pid) == 1)
		{
			strcat(path, entry->d_name);
			stat(path, &buf);
			if (buf.st_uid != getuid())
			{
				strcpy(path,"/proc/");
				continue;
			}

			//Gets CMDLINE
			strcpy(tmp, path);
			strcat(tmp, "/cmdline");
			FILE *fp = fopen(tmp, "r");
			if (fp == NULL) return 1;
			fgets(cmdline, 128,fp);
			fclose(fp);

			//Opens the stat file
			strcat(path, "/stat");
			fp = fopen(path, "r");
			if (fp == NULL) return 1;
			fgets(line, 128, fp);
			char *pch = strtok(line, " ");
			for (i = 0; i < 24; ++i)
			{
				strcpy(fields[i],pch);
				pch = strtok(NULL, " ");
			}


			//Checking for terminal
			if (atoi(fields[3]) != ppid)
			{
				strcpy(path,"/proc/");
				fclose(fp);
				continue;
			}

			float cpuTime = ComputeCPUTime(fields[13], fields[14], fields[15], fields[16]);
			total_cpu += cpuTime;

			//Converting VSZ
			vsize = atol(fields[22]);
			vsize /= 1000;
			total_size += vsize;
			printf("%s\t%s\t%4.2fs\t%s\t", fields[0], fields[3], cpuTime , fields[2]);
			PrintStartTime(fields[21]);
			printf("\t%ld\t%s\t%s\t\t%s\n", vsize, fields[23], fields[17], cmdline);
			strcpy(path,"/proc/");
			fclose(fp);
		}
	}
	closedir(directory);
	printf("---------------------------------\n");
	printf("Total memory usage Kb: %ld\n", total_size);
	printf("Total cpu time secs: %4.2f\n", total_cpu);
	return 0;
}

float ComputeCPUTime(char *time1, char *time2, char *time3, char* time4)
{
	float cpuTime = ((float)atoi(time1)/_HZ_) + ((float)atoi(time2)/_HZ_) +
	((float)atoi(time3)/_HZ_) + ((float)atoi(time4)/_HZ_);
	return cpuTime;
}

void PrintStartTime(char *startTime)
{
	unsigned long curtime = time(NULL);
	float uptime;
	struct tm tms;
	FILE *fp = fopen("/proc/uptime", "r");
	if (fp == NULL) return;
	fscanf(fp, "%f", &uptime);
	fclose(fp);
	curtime -= uptime;
	curtime += (float)atoi(startTime)/_HZ_;
	localtime_r(&curtime, &tms);
	printf("%02d:%02d", tms.tm_hour, tms.tm_min);
}
/*---------PS-----------*/

/*---------FT---------*/
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
							if ((entry->d_type == DT_LNK) && (atoi(entry->d_name) > 2))	// if it is a link other than standard iput/output/error streams
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
								
								// Each time we print for a new process, we print out its PID
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
					closedir(process);
				}
				break;
		}
	}
	closedir(proc);
	if (count)
		printf("-------------------\nTotal open files: %d\n", count);
	exit(EXIT_SUCCESS);
}

// Converts st_mode to human readable format of drwxr-x--x
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
/*---------FT---------*/

/*---------IOSTAT-----------*/
int iostat(int records, char *fieldname)
{
	struct stat buf;
	struct dirent *entry;
	DIR *directory;
	int pid;
	char path[128] = "/proc/";
	int i;
	FILE* fp;
	char word[32];
	int currentRecords = 0;

	ioinfo* ioarray;
	ioarray = malloc(records * sizeof(ioinfo));

	directory = opendir("/proc");
	printf("    PID\t\t   RCHAR\t  WCHAR\t\t  SYSR\t\t  SYSW\t\t  RFS\t\t  WFS\n");
	while ((entry = readdir(directory)) != NULL && currentRecords < records)
	{
		if (sscanf(entry->d_name, "%d", &pid) == 1)
		{
			strcat(path, entry->d_name);
			stat(path, &buf);
			if (buf.st_uid != getuid())
			{
				strcpy(path,"/proc/");
				continue;
			}

			//Opens the io file
			strcat(path, "/io");
			fp = fopen(path, "r");
			if (fp == NULL) return 1;

			ioarray[currentRecords].pid = pid;
			fscanf(fp, "%s %d", word, &ioarray[currentRecords].rchar);
			fscanf(fp, "%s %d", word, &ioarray[currentRecords].wchar);
			fscanf(fp, "%s %d", word, &ioarray[currentRecords].sysr);
			fscanf(fp, "%s %d", word, &ioarray[currentRecords].sysw);
			fscanf(fp, "%s %d", word, &ioarray[currentRecords].rfs);
			fscanf(fp, "%s %d", word, &ioarray[currentRecords].wfs);

			currentRecords++;
			strcpy(path,"/proc/");
			fclose(fp);
		}
	}

	//Sort the array based on given field name
	if (!strcmp(fieldname, "PID")) {
		qsort(ioarray, records, sizeof(ioinfo), cmpp);
	}
	else if (!strcmp(fieldname, "RCHAR")) {
		qsort(ioarray, records, sizeof(ioinfo), cmpr);
	}
	else if (!strcmp(fieldname, "WCHAR")) {
		qsort(ioarray, records, sizeof(ioinfo), cmpw);
	}
	else if (!strcmp(fieldname, "SYSR")) {
		qsort(ioarray, records, sizeof(ioinfo), cmpsr);
	}
	else if (!strcmp(fieldname, "SYSW")) {
		qsort(ioarray, records, sizeof(ioinfo), cmpsw);
	}
	else if (!strcmp(fieldname, "RFS")) {
		qsort(ioarray, records, sizeof(ioinfo), cmprf);
	}
	else if (!strcmp(fieldname, "WFS")) {
		qsort(ioarray, records, sizeof(ioinfo), cmpwf);
	}

	//Print the array
	for (i = 0; i < records; ++i)
	{
		printf("%8d\t%8d\t%8d\t%8d\t%8d\t%8d\t%8d\t\n", 
			ioarray[i].pid, ioarray[i].rchar,
			ioarray[i].wchar, ioarray[i].sysr,
			ioarray[i].sysw, ioarray[i].rfs,
			ioarray[i].wfs);
	}

	closedir(directory);
	free(ioarray);
	return 0;
}

int cmpp (const void * a, const void * b) {
	return ((*(ioinfo *)b).pid - (*(ioinfo *)a).pid); }

int cmpr (const void * a, const void * b) {
	return ((*(ioinfo *)b).rchar - (*(ioinfo *)a).rchar); }

int cmpw (const void * a, const void * b) {
	return ((*(ioinfo *)b).wchar - (*(ioinfo *)a).wchar); }

int cmpsr (const void * a, const void * b) {
	return ((*(ioinfo *)b).sysr - (*(ioinfo *)a).sysr); }

int cmpsw (const void * a, const void * b) {
	return ((*(ioinfo *)b).sysw - (*(ioinfo *)a).sysw); }

int cmprf (const void * a, const void * b) {
	return ((*(ioinfo *)b).rfs - (*(ioinfo *)a).rfs); }

int cmpwf (const void * a, const void * b) {
	return ((*(ioinfo *)b).wfs - (*(ioinfo *)a).wfs); }
/*---------IOSTAT-----------*/

/*---------NETSTAT---------*/
int netstat(const char *arg)
{
	struct stat stats;
	struct dirent *procentry, *entry;
	FILE *file;
	DIR *proc, *process;
	long my_pid, my_ppid, my_uid, my_gid, target_pid, target_ppid;
	char buffer[512], tmp[512], temp[512], permissions[11], laddress[62], raddress[62];
	int first, procnum, flagTCP = 1, flagUDP = 1, inode, sinode;

	// Setting which sockets to check for
	if (arg != NULL)
	{
		if (!strcmp(arg, "tcp"))
			flagUDP = 0;
		else
			flagTCP = 0;
	}

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

	// Print first line
	printf("%-9s  %-5s  %-12s  %-5s  %-12s %-5s\n", "PROTOCOL", "PID", "L-ADDRESS", "L-PORT", "R-ADDRESS", "R-PORT");

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

					// Finding open sockets for process
					strcat(tmp, "/fd");
					// Checking all contents of fd folder
					if ((process = opendir(tmp)) != NULL)
					{
						while ((entry = readdir(process)) != NULL)	// access only content allowed
						{
							if ((entry->d_type == DT_LNK) && (atoi(entry->d_name) > 2))	// We skip checking default input-output-error stream
							{
								inode = 0;
								strncpy(temp, "", sizeof(temp));
								strcpy(buffer, tmp);
								strcat(buffer, "/");
								strcat(buffer, entry->d_name);
								readlink(buffer, temp, sizeof(temp));

								// If we have found a socket
								if (!strncmp(temp, "socket", 6))
								{
									sscanf(temp, "socket:[%d]", &inode);	// We extract the inode number of the socket

									if (flagTCP)
									{
										file = fopen("/proc/net/tcp", "r");
										if (file == NULL) continue;

										// We search in the list of active sockets, the one that matches to the inode we have
										while (!feof(file))
										{
											fscanf(file, "%*s %s %s %*s %*s %*s %*s %*s %*s %d", laddress, raddress, &sinode);

											if (inode == sinode)
											{
												int local_address, remote_address, local_port, remote_port;
												char address[16];

												sscanf(laddress, "%x:%x", &local_address, &local_port);
												sscanf(raddress, "%x:%x", &remote_address, &remote_port);
												printf("%-9s  %-5ld  %-12s  %-5d  %-12s %-5d\n", "TCP", target_pid, decode_address(local_address, address), local_port, decode_address(remote_address, address), remote_port);
											}
										}
										fclose(file);
									}

									if (flagUDP)
									{
										file = fopen("/proc/net/udp", "r");
										if (file == NULL) continue;

										// We search in the list of active sockets, the one that matches to the inode we have
										while (!feof(file))
										{
											fscanf(file, "%*s %s %s %*s %*s %*s %*s %*s %*s %d", laddress, raddress, &sinode);

											if (inode == sinode)
											{
												int local_address, remote_address, local_port, remote_port;
												char address[16];

												sscanf(laddress, "%x:%x", &local_address, &local_port);
												sscanf(raddress, "%x:%x", &remote_address, &remote_port);
												printf("%-9s  %-5ld  %-12s  %-5d  %-12s %-5d\n", "UDP", target_pid, decode_address(local_address, address), local_port, decode_address(remote_address, address), remote_port);
											}
										}
										fclose(file);
									}
								}
							}
						}
					}
				}
				break;
		}
	}
	closedir(proc);
	exit(EXIT_SUCCESS);
}

// Decodes hex representation of ip inside tcp/udp file to a human-readable ip address
char *decode_address(int int_address, char *address)
{
	sprintf(address, "%d.%d.%d.%d", (int_address & 0x000000FF), (int_address & 0x0000FF00) >> 8, (int_address & 0x00FF0000) >> 16, (int_address & 0xFF000000) >> 24);
	return address;
}
/*---------NETSTAT---------*/