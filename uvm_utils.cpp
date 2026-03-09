#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <climits>
#include <dirent.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "uvm_utils.h"

int g_uvmfd = -1;

static int find_initialized_uvm()
{
	pid_t pid = getpid();
	char fd_dir[64];
	const char *target_path = "/dev/nvidia-uvm";
	int ret = -1;

	snprintf(fd_dir, sizeof(fd_dir), "/proc/%d/fd", (int)pid);

	DIR *dir = opendir(fd_dir);
	if (!dir)
		return ret;

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		char link_path[PATH_MAX];
		char fd_path[PATH_MAX];

		if (entry->d_name[0] == '.')
			continue;

		snprintf(link_path, sizeof(link_path), "%s/%s", fd_dir, entry->d_name);

		ssize_t len = readlink(link_path, fd_path, sizeof(fd_path) - 1);
		if (len < 0)
			continue;
		fd_path[len] = '\0';

		if (strcmp(fd_path, target_path) != 0)
			continue;

		int fd = atoi(entry->d_name);
		if (fd < 0)
			continue;

		UVM_PAGEABLE_MEM_ACCESS_PARAMS params = {};
		if (ioctl(fd, UVM_PAGEABLE_MEM_ACCESS, &params) == 0 &&
		    params.rmStatus == 0) {
			ret = fd;
			break;
		}
	}

	closedir(dir);
	return ret;
}

bool try_init_uvmfd()
{
	if (g_uvmfd >= 0)
		return true;

	g_uvmfd = find_initialized_uvm();
	if (g_uvmfd < 0)
		return false;

	printf("Found initialized uvmfd at %d\n", g_uvmfd);
	return true;
}
