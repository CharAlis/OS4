#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>

int cnt(const char *dirName, int flg);

int main(int argc, char const *argv[])
{
	cnt(argv[1], 0);
	return 0;
}

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
		n_dirs -= counter * 2;
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