#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>

#define _HZ_ 100

int ps();
float ComputeCPUTime(char *time1, char *time2, char *time3, char* time4);
void PrintStartTime(char *startTime);

int main(int argc, char const *argv[])
{
	ps();
	return 0;
}

int ps()
{
	struct stat buf;
	struct dirent *entry;
	DIR *directory;
	int pid, i;
	int ppid = getppid();
	directory = opendir("/proc");
	char path[128] = "/proc/";
	char line[128];
	char tmp[128];
	long int vsize;
	char cmdline[128];
	char fields[24][128];
	long total_size;
	float total_cpu;
	//MUST FIX: CPU + START
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
			printf("%s\t%s\t%4.2f\t%s\t", fields[0], fields[3], cpuTime , fields[2]);
			PrintStartTime(fields[21]);
			printf("\t%ld\t%s\t%s\t\t%s\n", vsize, fields[23], fields[17], cmdline);
			strcpy(path,"/proc/");
			fclose(fp);
		}
	}
	printf("---------------------------------\n");
	printf("Total memory usage Kb: %ld\n", total_size);
	printf("Total cpu time secs: %4.2f\n", total_cpu);
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
	gmtime_r(&curtime, &tms);
	printf("%d:%d", tms.tm_hour + 2, tms.tm_min);
}