#ifndef GVM_NOTIFY_H
#define GVM_NOTIFY_H

#include <pthread.h>
#include <cstdint>

#include <cuda.h>

enum gvm_notice_type_t
{
	GVM_NOTICE_EVICTION     = 0,
	GVM_NOTICE_AVAILABILITY = 1,
};

#define UVM_WAIT_NOTICE 82
#define NV_OK 0x00000000

struct UVM_WAIT_NOTICE_PARAMS
{
	CUuuid            uuid;
	gvm_notice_type_t type;
	union {
		struct {
			uint64_t target_memory;
			uint64_t current_memory;
		} eviction;
		struct {
			uint64_t available_memory;
		} availability;
	};
	int rmStatus;
};

typedef void (*gvm_notice_fn)(const UVM_WAIT_NOTICE_PARAMS *params);

int  gvm_register_notify(gvm_notice_fn handler);
void gvm_unregister_notify();

#endif
