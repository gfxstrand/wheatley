#include "jni_util.h"

void
jni_util_throw_by_name(JNIEnv *env, const char *name, const char *message)
{
    jclass cls;

    cls = (*env)->FindClass(env, name);
    if (!cls)
        return;

    (*env)->ThrowNew(env, cls, message);

    (*env)->DeleteLocalRef(env, cls);
}
