#define _HZ_ 100

/*---------CNT-----------*/
int cnt(const char *dirName, int flg);
/*---------CNT-----------*/

/*---------LISTRW---------*/
int listrw(const char *type, int flag, const char *strtime, const char *dir);

int findpos(const char *string, char c);
void check(const char *dir, time_t curtime, long timelimit, int flag, struct dirent *entry, struct stat *pstats);
/*---------LISTRW---------*/

/*---------PS-----------*/
int ps();

float ComputeCPUTime(char *time1, char *time2, char *time3, char* time4);
void PrintStartTime(char *startTime);
/*---------PS-----------*/

/*---------FT---------*/
int ft();

void convertmode(int mode, char *perms);
/*---------FT---------*/

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

/*---------NETSTAT---------*/
int netstat(const char *arg);

char *decode_address(int int_address, char *address);
/*---------NETSTAT---------*/