#ifndef UVM_UTILS_H
#define UVM_UTILS_H

#include <cstdint>
#include <sys/ioctl.h>
#include <cuda.h>

#define UVM_IOCTL_BASE(i) (i)

#define UVM_INITIALIZE          0x30000001
#define UVM_REGISTER_GPU        UVM_IOCTL_BASE(37)
#define UVM_PAGEABLE_MEM_ACCESS UVM_IOCTL_BASE(39)

#define NV_IOCTL_MAGIC          'F'
#define NV_ESC_RM_ALLOC         0x2B
#define NV01_ROOT_CLIENT        0x00000041

struct UVM_INITIALIZE_PARAMS {
	uint64_t flags;
	int      rmStatus;
};

struct UVM_REGISTER_GPU_PARAMS {
	CUuuid   gpu_uuid;
	uint8_t  numaEnabled;
	int32_t  numaNodeId;
	int32_t  rmCtrlFd;
	uint32_t hClient;
	uint32_t hSmcPartRef;
	int      rmStatus;
};

struct UVM_PAGEABLE_MEM_ACCESS_PARAMS {
	uint32_t pageableMemAccess;
	int      rmStatus;
};

struct NVOS64_PARAMETERS {
	uint32_t hRoot;
	uint32_t hObjectParent;
	uint32_t hObjectNew;
	uint32_t hClass;
	uint64_t pAllocParms      __attribute__((aligned(8)));
	uint64_t pRightsRequested __attribute__((aligned(8)));
	uint32_t paramsSize;
	uint32_t flags;
	uint32_t status;
};

extern int g_uvmfd;
int  init_uvmfd(void);

#endif
