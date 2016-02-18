#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <stdlib.h>
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