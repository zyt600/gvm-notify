# gvm-notify

Lightweight library for receiving GVM eviction/availability notifications from the UVM kernel driver. Provides both C/C++ (`libgvmnotify.so`) and Python bindings.

## Build & Install

```bash
# C/C++ shared library
sudo make install-cxx

# Python package
make install-python
```

## Usage

**C/C++** — link against `-lgvmnotify`:

```c
#include <gvm_notify.h>

void handler(const UVM_WAIT_NOTICE_PARAMS *p) { /* ... */ }

gvm_register_notify(handler);
// ...
gvm_unregister_notify();
```

**Python:**

```python
import gvm_notify

def on_notice(info):
    print(info)  # dict with uuid, type, rmStatus, ...

gvm_notify.register(on_notice)
# ...
gvm_notify.unregister()
```

## Uninstall

```bash
sudo make uninstall-cxx
make uninstall-python
```
