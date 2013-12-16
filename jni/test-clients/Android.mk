LOCAL_PATH := $(call my-dir)

# Protocol stuff
WAYLAND_SCANNER_SRC := $(LIBWLB_PATH)/protocol/fullscreen-shell.xml
WAYLAND_SCANNER_DEST := $(LOCAL_PATH)
include $(WHEATLEY_PATH)/external/wayland-scanner.mk

include $(CLEAR_VARS)

LOCAL_MODULE		:= simple-shm
LOCAL_SRC_FILES		:=		\
	fullscreen-shell-protocol.c	\
	os-compatibility.c		\
	simple-shm.c
LOCAL_STATIC_LIBRARIES	:= libwayland-client

simple-shm: wayland-client $(LOCAL_PATH)/fullscreen-shell-client-protocol.h

include $(BUILD_EXECUTABLE)
