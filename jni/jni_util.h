#ifndef __WHEATLEY_JNI_UTIL_H__
#define __WHEATLEY_JNI_UTIL_H__

#include <jni.h>

void
jni_util_throw_by_name(JNIEnv *env, const char *name, const char *message);

#endif /* ! defined __WHEATLEY_JNI_UTIL_H__ */
