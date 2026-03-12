#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <cstring>

#include "gvm_notify.h"

namespace py = pybind11;

static py::object g_py_callback;

static void trampoline(const UVM_WAIT_NOTICE_PARAMS *params) {
	py::gil_scoped_acquire gil;
	if (!g_py_callback)
		return;

	py::dict info;
	info["uuid"] = py::bytes(params->uuid.bytes, 16);
	info["type"] = static_cast<int>(params->type);
	info["rmStatus"] = params->rmStatus;

	if (params->type == GVM_NOTICE_EVICTION) {
		info["target_memory"] = params->eviction.target_memory;
		info["current_memory"] = params->eviction.current_memory;
	} else if (params->type == GVM_NOTICE_AVAILABILITY) {
		info["available_memory"] = params->availability.available_memory;
	}

	try {
		g_py_callback(info);
	} catch (py::error_already_set &e) {
		e.restore();
		PyErr_Print();
	}
}

static void py_register(py::object callback) {
	if (g_py_callback)
		throw std::runtime_error("already registered, call unregister() first");

	g_py_callback = std::move(callback);

	py::gil_scoped_release release;
	if (gvm_register_notify(trampoline) != 0) {
		g_py_callback = py::object();
		throw std::runtime_error("gvm_register_notify failed");
	}
}

static void py_unregister() {
	{
		py::gil_scoped_release release;
		gvm_unregister_notify();
	}
	g_py_callback = py::object();
}

PYBIND11_MODULE(gvm_notify, m) {
	m.doc() = "GVM notification listener";

	m.attr("EVICTION") = static_cast<int>(GVM_NOTICE_EVICTION);
	m.attr("AVAILABILITY") = static_cast<int>(GVM_NOTICE_AVAILABILITY);

	m.def("register", &py_register, py::arg("callback"),
	      "Register a callback to receive GVM notices. "
	      "The callback receives a dict with keys: uuid, type, rmStatus, "
	      "and type-specific fields (target_memory/current_memory or available_memory).");

	m.def("unregister", &py_unregister,
	      "Stop the notification listener and release the callback.");
}
