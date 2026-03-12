CUDA_PATH ?= /usr/local/cuda
BUILD ?= build
PREFIX ?= /usr/local
INCLUDEDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib

CXX = g++
CXXFLAGS = -Wall -fPIC -std=c++17 -I$(CUDA_PATH)/include
LDFLAGS = -shared -lpthread -L$(CUDA_PATH)/lib64 -lcuda

SRCS = gvm_notify.cpp uvm_utils.cpp
OBJS = $(addprefix $(BUILD)/,$(SRCS:.cpp=.o))
TARGET = $(BUILD)/libgvmnotify.so

all: $(TARGET)

install-cxx: $(TARGET)
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -d $(DESTDIR)$(LIBDIR)
	install -m 644 gvm_notify.h  $(DESTDIR)$(INCLUDEDIR)/
	install -m 755 $(TARGET)     $(DESTDIR)$(LIBDIR)/
	ldconfig 2>/dev/null || true

install-python:
	pip install ./python

uninstall-cxx:
	rm -f  $(DESTDIR)$(INCLUDEDIR)/gvm_notify.h
	rm -f  $(DESTDIR)$(LIBDIR)/libgvmnotify.so
	ldconfig 2>/dev/null || true

uninstall-python:
	pip uninstall -y gvm-notify

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
	rm -rf python/build python/*.egg-info

.PHONY: all install-cxx install-python uninstall-cxx uninstall-python clean
