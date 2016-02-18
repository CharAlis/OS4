#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <sys/times.h>
#include <stdlib.h>
#include "functions.h"

int main(int argc, char const *argv[])
{
	iostat(3, "PID");
	return 0;
}

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