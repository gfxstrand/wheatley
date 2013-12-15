LOCAL_PATH := $(call my-dir)

LIBWLB_PATH := $(LOCAL_PATH)/libwlb

# Protocol stuff
wayland_protocoldir := $(LIBWLB_PATH)/protocol
include $(LOCAL_PATH)/wayland-scanner.mk

LOCAL_PATH := $(LIBWLB_PATH)/libwlb

include $(CLEAR_VARS)

LOCAL_MODULE 	:= libwlb
LOCAL_SRC_FILES	:=			\
	fullscreen-shell-protocol.c	\
	util.c				\
	matrix.c			\
	surface.c			\
	output.c			\
	seat.c				\
	pointer.c			\
	keyboard.c			\
	pixman-renderer.c		\
	glesv2-renderer.c		\
	compositor.c
LOCAL_GENERATED_SOURCES	:= fullscreen-shell-protocol.c
LOCAL_LDLIBS		:= -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES	:= libpixman libwayland-server

include $(BUILD_SHARED_LIBRARY)

