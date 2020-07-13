#include <jni.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES/gl.h>

extern void xmit(EGLDisplay display, EGLSurface surface, EGLContext context,int setdata);
extern void rd_init(ANativeWindow *window, int onestep);
extern void rd_start();
extern void rd_step();
extern void rd_stop();
extern EGLDisplay getEGLDisplay();
extern EGLSurface getEGLSurface();
extern EGLContext getEGLContext();


static ANativeWindow *window = 0;
static char app_ptr[32]= {0};


JNIEXPORT void JNICALL
Java_{{ cookiecutter.bundle|replace('.', '_') }}_{{ cookiecutter.module_name }}_MainActivity_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface)
{

    EGLint width;
    EGLint height;

    LOG_V("jni.nativeSetSurface");

    if ( !surface ){
        LOG_V("jni.nativeSetSurface->Releasing window");
        if (window)
            ANativeWindow_release(window);
        return;
    }

    if (getenv("PANDA_NATIVE_WINDOW")) {
        LOG_E("jni.nativeSetSurface-> native window already set");
        return;
    }

    window = ANativeWindow_fromSurface(jenv, surface);
    LOG_V("jni.nativeSetSurface-> NATIVE_WINDOW %p", window);

    snprintf(app_ptr, 16, "%p", (void * )window );
    setenv("PANDA_NATIVE_WINDOW", app_ptr, 1);

    return;
}








