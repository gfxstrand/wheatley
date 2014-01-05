LOCAL_PATH := $(call my-dir)

LIBWLB_PATH := $(LOCAL_PATH)/libwlb

# Protocol stuff
WAYLAND_SCANNER_SRC := $(LIBWLB_PATH)/protocol/fullscreen-shell.xml
WAYLAND_SCANNER_DEST := $(LIBWLB_PATH)/libwlb
include $(WHEATLEY_PATH)/external/wayland-scanner.mk

LOCAL_PATH := $(LIBWLB_PATH)/libwlb

wlb: wayland-server

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
	touch.c				\
	keyboard.c			\
	pixman-renderer.c		\
	glesv2-renderer.c		\
	wayland-egl-binding.c		\
	compositor.c
LOCAL_GENERATED_SOURCES	:= fullscreen-shell-protocol.c
LOCAL_LDLIBS		:= -lEGL -lGLESv2
LOCAL_EXPORT_C_INCLUDES := $(LIBWLB_PATH)/libwlb
LOCAL_STATIC_LIBRARIES	:= libpixman libwayland-server

wlb: wayland-server $(LOCAL_PATH)/fullscreen-shell-server-protocol.h

include $(BUILD_STATIC_LIBRARY)

