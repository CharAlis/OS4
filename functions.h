#define _HZ_ 100

/*---------CNT-----------*/
int cnt(const char *dirName, int flg);
/*---------CNT-----------*/

/*---------PS-----------*/
int ps();

float ComputeCPUTime(char *time1, char *time2, char *time3, char* time4);
void PrintStartTime(char *startTime);
/*---------PS-----------*/

/*---------IOSTAT-----------*/

//Struct that keeps the i/o information for a process
typedef struct ioinfo
{
	int pid;
	int rchar;
	int wchar;
	int sysr;
	int sysw;
	int rfs;
	int wfs;
} ioinfo;

int iostat(int records, char *fieldname);

//Compare functions for quicksort
int cmpp (const void *, const void *); //Order by pid
int cmpr (const void *, const void *); //Order by rchar
int cmpw (const void *, const void *); //Order by wchar
int cmpsr (const void *, const void *); //Order by sysr
int cmpsw (const void *, const void *); //Order by sysw
int cmprf (const void *, const void *); //Order by rfs
int cmpwf (const void *, const void *); //Order by wfs
/*---------IOSTAT-----------*/