#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <unistd.h>
#include <climits>
#include <sys/ioctl.h>
#include <cuda.h>

#include "uvm_utils.h"

#define UVM_IS_INITIALIZED 80

struct UVM_IS_INITIALIZED_PARAMS {
	CUuuid uuid;
	bool   initialized;
	int    rmStatus;
};

int g_uvmfd = -1;

static int get_gpu_uuid(CUuuid *uuid)
{
	CUdevice device;

	if (cuInit(0) != CUDA_SUCCESS) {
		fprintf(stderr, "get_gpu_uuid: cuInit failed\n");
		return -1;
	}
	if (cuDeviceGet(&device, 0) != CUDA_SUCCESS) {
		fprintf(stderr, "get_gpu_uuid: cuDeviceGet failed\n");
		return -1;
	}
	if (cuDeviceGetUuid(uuid, device) != CUDA_SUCCESS) {
		fprintf(stderr, "get_gpu_uuid: cuDeviceGetUuid failed\n");
		return -1;
	}
	return 0;
}

static int find_initialized_uvm(CUuuid uuid)
{
	pid_t pid = getpid();
	char fd_dir[64];
	const char *target_path = "/dev/nvidia-uvm";
	int ret = -1;

	snprintf(fd_dir, sizeof(fd_dir), "/proc/%d/fd", (int)pid);

	DIR *dir = opendir(fd_dir);
	if (!dir) {
		fprintf(stderr, "Failed to open %s: %s\n", fd_dir, strerror(errno));
		return ret;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		char link_path[PATH_MAX];
		char fd_path[PATH_MAX];
		ssize_t len;
		int fd;

		if (entry->d_name[0] == '.' &&
			(entry->d_name[1] == '\0' ||
			 (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
			continue;
		}

		snprintf(link_path, sizeof(link_path), "%s/%s", fd_dir, entry->d_name);

		len = readlink(link_path, fd_path, sizeof(fd_path) - 1);
		if (len < 0)
			continue;
		fd_path[len] = '\0';

		if (strcmp(fd_path, target_path) != 0)
			continue;

		fd = atoi(entry->d_name);
		if (fd < 0) {
			fprintf(stderr, "Invalid file descriptor: %d\n", fd);
			continue;
		}

		UVM_IS_INITIALIZED_PARAMS params = {};
		memcpy(&params.uuid, &uuid, sizeof(CUuuid));
		params.initialized = false;
		params.rmStatus = 0;

		if (ioctl(fd, UVM_IS_INITIALIZED, &params) == 0 && params.rmStatus == 0) {
			if (params.initialized) {
				ret = fd;
				break;
			}
		} else {
			fprintf(stderr, "Failed to check UVM initialization on fd %d: %s\n",
					fd, strerror(errno));
		}
	}

	closedir(dir);
	return ret;
}

int init_uvmfd(void)
{
	if (g_uvmfd >= 0)
		return 0;

	CUuuid gpu_uuid;
	if (get_gpu_uuid(&gpu_uuid) != 0)
		return -1;

	g_uvmfd = find_initialized_uvm(gpu_uuid);
	if (g_uvmfd < 0) {
		fprintf(stderr, "init_uvmfd: no initialized UVM fd found\n");
		return -1;
	}

	printf("Found initialized uvmfd=%d\n", g_uvmfd);
	return 0;
}
