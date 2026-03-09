#ifndef UVM_UTILS_H
#define UVM_UTILS_H

#include <cstdint>

#define UVM_IOCTL_BASE(i) (i)

#define UVM_PAGEABLE_MEM_ACCESS UVM_IOCTL_BASE(39)

struct UVM_PAGEABLE_MEM_ACCESS_PARAMS {
	uint32_t pageableMemAccess;
	int      rmStatus;
};

extern int g_uvmfd;
bool try_init_uvmfd();

#endif
