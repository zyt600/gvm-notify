#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>

#include "gvm_notify.h"
#include "uvm_utils.h"

static gvm_notice_fn g_handler;
static volatile bool g_active;
static pthread_t     g_notify_thread;

static void *notify_thread_fn(void *) {
	UVM_WAIT_NOTICE_PARAMS params;

	while (g_active) {
		memset(&params, 0, sizeof(params));
		if (ioctl(g_uvmfd, UVM_WAIT_NOTICE, &params) != 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		g_handler(&params);
	}
	return nullptr;
}

int gvm_register_notify(gvm_notice_fn handler) {
	if (init_uvmfd() != 0) {
		fprintf(stderr, "gvm_register_notify: init_uvmfd failed\n");
		return -1;
	}

	g_handler = handler;
	g_active = true;

	if (pthread_create(&g_notify_thread, nullptr, notify_thread_fn, nullptr) != 0) {
		g_active = false;
		perror("gvm_register_notify: thread create failed");
		return -1;
	}
	return 0;
}

void gvm_unregister_notify() {
	g_active = false;
	pthread_cancel(g_notify_thread);
	pthread_join(g_notify_thread, nullptr);
}
