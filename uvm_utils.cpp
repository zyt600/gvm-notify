#include <cstdio>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "uvm_utils.h"

int g_uvmfd = -1;

static int rm_alloc_client(int ctlfd, uint32_t *hClient)
{
	NVOS64_PARAMETERS params = {};
	params.hClass = NV01_ROOT_CLIENT;

	if (ioctl(ctlfd, _IOWR(NV_IOCTL_MAGIC, NV_ESC_RM_ALLOC, NVOS64_PARAMETERS), &params) != 0) {
		perror("NV_ESC_RM_ALLOC");
		return -1;
	}
	if (params.status != 0) {
		fprintf(stderr, "NV_ESC_RM_ALLOC failed: status=0x%x\n", params.status);
		return -1;
	}

	*hClient = params.hObjectNew;
	return 0;
}

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

int init_uvmfd(void)
{
	if (g_uvmfd >= 0)
		return 0;

	CUuuid gpu_uuid;
	if (get_gpu_uuid(&gpu_uuid) != 0)
		return -1;

	int uvmfd = open("/dev/nvidia-uvm", O_RDWR);
	if (uvmfd < 0) {
		perror("open /dev/nvidia-uvm");
		return -1;
	}

	UVM_INITIALIZE_PARAMS init_params = {};
	if (ioctl(uvmfd, UVM_INITIALIZE, &init_params) != 0 || init_params.rmStatus != 0) {
		fprintf(stderr, "UVM_INITIALIZE failed: ioctl=%s, rmStatus=%d\n",
			strerror(errno), init_params.rmStatus);
		close(uvmfd);
		return -1;
	}

	int ctlfd = open("/dev/nvidiactl", O_RDWR);
	if (ctlfd < 0) {
		perror("open /dev/nvidiactl");
		close(uvmfd);
		return -1;
	}

	uint32_t hClient = 0;
	if (rm_alloc_client(ctlfd, &hClient) != 0) {
		close(ctlfd);
		close(uvmfd);
		return -1;
	}

	UVM_REGISTER_GPU_PARAMS reg_params = {};
	memcpy(&reg_params.gpu_uuid, &gpu_uuid, sizeof(CUuuid));
	reg_params.rmCtrlFd = ctlfd;
	reg_params.hClient = hClient;
	reg_params.hSmcPartRef = 0;

	if (ioctl(uvmfd, UVM_REGISTER_GPU, &reg_params) != 0 || reg_params.rmStatus != 0) {
		fprintf(stderr, "UVM_REGISTER_GPU failed: ioctl=%s, rmStatus=%d\n",
			strerror(errno), reg_params.rmStatus);
		close(ctlfd);
		close(uvmfd);
		return -1;
	}

	printf("Self-initialized uvmfd=%d, ctlfd=%d, hClient=0x%x\n", uvmfd, ctlfd, hClient);

	g_uvmfd = uvmfd;
	return 0;
}
