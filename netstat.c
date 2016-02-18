#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

char *decode_address(int int_address, char *address);

int main(int argc, char const *argv[])
{
	if (argc >= 2)
		netstat(argv[1]);
	else
		netstat(NULL);
	exit(EXIT_SUCCESS);
}

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
							if ((entry->d_type == DT_LNK) && (atoi(entry->d_name) > 2))
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
									sscanf(temp, "socket:[%d]", &inode);
									//printf("found socket '%s', it has inode of '%d'\n", temp, inode);
									//printf("flagTCP : %d, flagUDP : %d\n", flagTCP, flagUDP);

									if (flagTCP)
									{
										file = fopen("/proc/net/tcp", "r");
										if (file == NULL) continue;

										while (!feof(file))
										{
											fscanf(file, "%*s %s %s %*s %*s %*s %*s %*s %*s %d", laddress, raddress, &sinode);
											//printf("laddress : '%s', raddress : '%s', sinode : '%d' \n", laddress, raddress, sinode);
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

										while (!feof(file))
										{
											fscanf(file, "%*s %s %s %*s %*s %*s %*s %*s %*s %d", laddress, raddress, &sinode);
											//printf("laddress : '%s', raddress : '%s', sinode : '%d' \n", laddress, raddress, sinode);
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
	exit(EXIT_SUCCESS);
}

char *decode_address(int int_address, char *address)
{
	sprintf(address, "%d.%d.%d.%d", (int_address & 0x000000FF), (int_address & 0x0000FF00) >> 8, (int_address & 0x00FF0000) >> 16, (int_address & 0xFF000000) >> 24);
	return address;
}