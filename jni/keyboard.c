/*
 * Copyright © 2014 Jason Ekstrand
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#define LOG_TAG "wayland.Keyboard"
#include "wheatley.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <android/input.h>
#include <linux/input.h>

static uint32_t convert_keycode(jint keycode);

JNIEXPORT jlong JNICALL
Java_net_jlekstrand_wheatley_wayland_Keyboard_createNative(JNIEnv *env,
        jclass cls, jlong seatHandle)
{
    struct wlb_seat *seat = (struct wlb_seat *)(intptr_t)seatHandle;
    struct wlb_keyboard *keyboard;

    keyboard = wlb_keyboard_create(seat);
    if (keyboard == NULL) {
        ALOGD("wlb_keyboard_create failed: %s\n", strerror(errno));
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to create Keyboard");
    }

    return (jlong)(intptr_t)keyboard;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Keyboard_destroyNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wlb_keyboard *keyboard =
            (struct wlb_keyboard *)(intptr_t)nativeHandle;

    wlb_keyboard_destroy(keyboard);
}

JNIEXPORT jboolean JNICALL
Java_net_jlekstrand_wheatley_wayland_Keyboard_handleKeyNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint time, jint keyCode,
        jboolean pressed)
{
    struct wlb_keyboard *keyboard =
            (struct wlb_keyboard *)(intptr_t)nativeHandle;
    uint32_t key;

    key = convert_keycode(keyCode);
    if (key == 0) {
        ALOGD("Unknown keycode: %d", keyCode);
        return JNI_FALSE;
    }

    wlb_keyboard_key(keyboard, time, key,
            pressed ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);

    return JNI_TRUE;
}

static uint32_t
convert_keycode(jint keycode)
{
    /* Let's hope our compiler decides to optimize this */
    switch (keycode) {
    case AKEYCODE_HOME: return KEY_HOMEPAGE;
    case AKEYCODE_BACK: return KEY_BACK;

    case AKEYCODE_0: return KEY_0;
    case AKEYCODE_1: return KEY_1;
    case AKEYCODE_2: return KEY_2;
    case AKEYCODE_3: return KEY_3;
    case AKEYCODE_4: return KEY_4;
    case AKEYCODE_5: return KEY_5;
    case AKEYCODE_6: return KEY_6;
    case AKEYCODE_7: return KEY_7;
    case AKEYCODE_8: return KEY_8;
    case AKEYCODE_9: return KEY_9;

    case AKEYCODE_DPAD_UP: return KEY_UP;
    case AKEYCODE_DPAD_DOWN: return KEY_DOWN;
    case AKEYCODE_DPAD_LEFT: return KEY_LEFT;
    case AKEYCODE_DPAD_RIGHT: return KEY_RIGHT;

    case AKEYCODE_VOLUME_UP: return KEY_VOLUMEUP;
    case AKEYCODE_VOLUME_DOWN: return KEY_VOLUMEDOWN;

    case AKEYCODE_CLEAR: return KEY_CLEAR;
    case AKEYCODE_A: return KEY_A;
    case AKEYCODE_B: return KEY_B;
    case AKEYCODE_C: return KEY_C;
    case AKEYCODE_D: return KEY_D;
    case AKEYCODE_E: return KEY_E;
    case AKEYCODE_F: return KEY_F;
    case AKEYCODE_G: return KEY_G;
    case AKEYCODE_H: return KEY_H;
    case AKEYCODE_I: return KEY_I;
    case AKEYCODE_J: return KEY_J;
    case AKEYCODE_K: return KEY_K;
    case AKEYCODE_L: return KEY_L;
    case AKEYCODE_M: return KEY_M;
    case AKEYCODE_N: return KEY_N;
    case AKEYCODE_O: return KEY_O;
    case AKEYCODE_P: return KEY_P;
    case AKEYCODE_Q: return KEY_Q;
    case AKEYCODE_R: return KEY_R;
    case AKEYCODE_S: return KEY_S;
    case AKEYCODE_T: return KEY_T;
    case AKEYCODE_U: return KEY_U;
    case AKEYCODE_V: return KEY_V;
    case AKEYCODE_W: return KEY_W;
    case AKEYCODE_X: return KEY_X;
    case AKEYCODE_Y: return KEY_Y;
    case AKEYCODE_Z: return KEY_Z;
    case AKEYCODE_COMMA: return KEY_COMMA;
    case AKEYCODE_PERIOD: return KEY_DOT;
    case AKEYCODE_ALT_LEFT: return KEY_LEFTALT;
    case AKEYCODE_ALT_RIGHT: return KEY_RIGHTALT;
    case AKEYCODE_SHIFT_LEFT: return KEY_LEFTSHIFT;
    case AKEYCODE_SHIFT_RIGHT: return KEY_RIGHTSHIFT;
    case AKEYCODE_TAB: return KEY_TAB;
    case AKEYCODE_SPACE: return KEY_SPACE;

    case AKEYCODE_EXPLORER: return KEY_WWW;
    case AKEYCODE_ENVELOPE: return KEY_EMAIL;
    case AKEYCODE_ENTER: return KEY_ENTER;
    case AKEYCODE_DEL: return KEY_BACKSPACE;
    case AKEYCODE_GRAVE: return KEY_GRAVE;
    case AKEYCODE_MINUS: return KEY_MINUS;
    case AKEYCODE_EQUALS: return KEY_EQUAL;
    case AKEYCODE_LEFT_BRACKET: return KEY_LEFTBRACE;
    case AKEYCODE_RIGHT_BRACKET: return KEY_RIGHTBRACE;
    case AKEYCODE_BACKSLASH: return KEY_BACKSLASH;
    case AKEYCODE_SEMICOLON: return KEY_SEMICOLON;
    case AKEYCODE_APOSTROPHE: return KEY_APOSTROPHE;
    case AKEYCODE_SLASH: return KEY_SLASH;

    case AKEYCODE_MENU: return KEY_MENU;

    case AKEYCODE_SEARCH: return KEY_SEARCH;
    case AKEYCODE_MEDIA_PLAY_PAUSE: return KEY_PLAYPAUSE;
    case AKEYCODE_MEDIA_STOP: return KEY_STOP;
    case AKEYCODE_MEDIA_NEXT: return KEY_NEXTSONG;
    case AKEYCODE_MEDIA_PREVIOUS: return KEY_PREVIOUSSONG;
    case AKEYCODE_MEDIA_REWIND: return KEY_REWIND;
    case AKEYCODE_MEDIA_FAST_FORWARD: return KEY_FASTFORWARD;

    case AKEYCODE_PAGE_UP: return KEY_PAGEUP;
    case AKEYCODE_PAGE_DOWN: return KEY_PAGEDOWN;

    case AKEYCODE_ESCAPE: return KEY_ESC;
    case AKEYCODE_FORWARD_DEL: return KEY_DELETE;
    case AKEYCODE_CTRL_LEFT: return KEY_LEFTCTRL;
    case AKEYCODE_CTRL_RIGHT: return KEY_RIGHTCTRL;
    case AKEYCODE_CAPS_LOCK: return KEY_CAPSLOCK;
    case AKEYCODE_SCROLL_LOCK: return KEY_SCROLLLOCK;
    case AKEYCODE_META_LEFT: return KEY_LEFTMETA;
    case AKEYCODE_META_RIGHT: return KEY_RIGHTMETA;
    case AKEYCODE_FUNCTION: return KEY_FN;
    case AKEYCODE_SYSRQ: return KEY_SYSRQ;
    case AKEYCODE_BREAK: return KEY_BREAK;
    case AKEYCODE_MOVE_HOME: return KEY_HOME;
    case AKEYCODE_MOVE_END: return KEY_END;
    case AKEYCODE_INSERT: return KEY_INSERT;
    case AKEYCODE_FORWARD: return KEY_FORWARD;
    case AKEYCODE_MEDIA_PLAY: return KEY_PLAYCD;
    case AKEYCODE_MEDIA_PAUSE: return KEY_PAUSECD;
    case AKEYCODE_MEDIA_CLOSE: return KEY_CLOSECD;
    case AKEYCODE_MEDIA_EJECT: return KEY_EJECTCD;
    case AKEYCODE_MEDIA_RECORD: return KEY_RECORD;
    case AKEYCODE_F1: return KEY_F1;
    case AKEYCODE_F2: return KEY_F2;
    case AKEYCODE_F3: return KEY_F3;
    case AKEYCODE_F4: return KEY_F4;
    case AKEYCODE_F5: return KEY_F5;
    case AKEYCODE_F6: return KEY_F6;
    case AKEYCODE_F7: return KEY_F7;
    case AKEYCODE_F8: return KEY_F8;
    case AKEYCODE_F9: return KEY_F9;
    case AKEYCODE_F10: return KEY_F10;
    case AKEYCODE_F11: return KEY_F11;
    case AKEYCODE_F12: return KEY_F12;
    case AKEYCODE_NUM_LOCK: return KEY_NUMLOCK;
    case AKEYCODE_NUMPAD_0: return KEY_KP0;
    case AKEYCODE_NUMPAD_1: return KEY_KP1;
    case AKEYCODE_NUMPAD_2: return KEY_KP2;
    case AKEYCODE_NUMPAD_3: return KEY_KP3;
    case AKEYCODE_NUMPAD_4: return KEY_KP4;
    case AKEYCODE_NUMPAD_5: return KEY_KP5;
    case AKEYCODE_NUMPAD_6: return KEY_KP6;
    case AKEYCODE_NUMPAD_7: return KEY_KP7;
    case AKEYCODE_NUMPAD_8: return KEY_KP8;
    case AKEYCODE_NUMPAD_9: return KEY_KP9;
    case AKEYCODE_NUMPAD_DIVIDE: return KEY_KPSLASH;
    case AKEYCODE_NUMPAD_MULTIPLY: return KEY_KPASTERISK;
    case AKEYCODE_NUMPAD_SUBTRACT: return KEY_KPMINUS;
    case AKEYCODE_NUMPAD_ADD: return KEY_KPPLUS;
    case AKEYCODE_NUMPAD_DOT: return KEY_KPDOT;
    case AKEYCODE_NUMPAD_COMMA: return KEY_KPCOMMA;
    case AKEYCODE_NUMPAD_ENTER: return KEY_KPENTER;
    case AKEYCODE_NUMPAD_EQUALS: return KEY_KPEQUAL;
    case AKEYCODE_NUMPAD_LEFT_PAREN: return KEY_KPLEFTPAREN;
    case AKEYCODE_NUMPAD_RIGHT_PAREN: return KEY_KPRIGHTPAREN;
    case AKEYCODE_VOLUME_MUTE: return KEY_MUTE;
    case AKEYCODE_INFO: return KEY_INFO;
    case AKEYCODE_CHANNEL_UP: return KEY_CHANNELUP;
    case AKEYCODE_CHANNEL_DOWN: return KEY_CHANNELDOWN;

    default: return KEY_RESERVED;
    }
}

/* vim: set ts=4 sw=4 sts=4 expandtab: */
