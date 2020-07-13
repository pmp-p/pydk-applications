//
// Copyright 2011 Tero Saarni
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <pthread.h>
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>

bool renderingEnabled = true;

class Renderer {

public:
    Renderer();
    virtual ~Renderer();

    // android window, supported by NDK r5 and newer
    ANativeWindow* window;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;


    // Following methods cannot be called from any thread.
    // They would need to send message to render thread which executes required actions.
    bool initialize();
    void step();

    // those are ok
    void setWindow(ANativeWindow* window);
    void start();
    void stop();


private:

    enum RenderThreadMessage {
        MSG_NONE = 0,
        MSG_WINDOW_SET,
        MSG_RENDER_LOOP_EXIT
    };

    pthread_t _threadId;
    pthread_mutex_t _mutex;
    enum RenderThreadMessage _msg;

    GLfloat _angle;

    // RenderLoop is called in a rendering thread started in start() method
    // It creates rendering context and renders scene until stop() is called
    void renderLoop();

    void destroy();

    void drawFrame();

    // Helper method for starting the thread
    static void* threadStartCallback(void *myself);

};

#define ATEST 0
#define GLES2 1

extern "C" {
    static Renderer *rd = new Renderer();



    EGLDisplay getEGLDisplay() {
        return rd->display;
    }
    EGLSurface getEGLSurface() {
        return rd->surface;
    }
    EGLContext getEGLContext() {
        return rd->context;
    }

    void xmit(EGLDisplay display, EGLSurface surface, EGLContext context, int setdata) {
        EGLint width;
        EGLint height;
        char app_ptr[32]= {0};

        if (setdata) {
            rd->display = display;
            rd->surface = surface;
            rd->context = context;
        }

        //TRANSMIT
        snprintf(app_ptr, 16, "%p", (void * )display );
        setenv("PANDA_NATIVE_DISPLAY", app_ptr, 1);
        LOG_V("jni.nativeSetSurface-> NATIVE_DISPLAY %s", app_ptr);

        //TRANSMIT
        snprintf(app_ptr, 16, "%p", (void * )surface );
        setenv("PANDA_NATIVE_SURFACE", app_ptr, 1);
        LOG_V("jni.nativeSetSurface-> NATIVE_SURFACE %s", app_ptr);

        context = getEGLContext();
        //TRANSMIT
        snprintf(app_ptr, 16, "%p", (void * )context );
        setenv("PANDA_NATIVE_CONTEXT", app_ptr, 1);
        LOG_V("jni.nativeSetSurface-> NATIVE_CONTEXT %s", app_ptr);


        if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
            !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
            LOG_E("jni.nativeSetSurface-> window.eglQuerySurface() returned error %d", eglGetError());
        } else
            LOG_I("mod_egl : window.egl dim %d x %d",width,height);
    }



    void rd_init(ANativeWindow *window, int onestep) {
        static EGLDisplay display;
        rd->setWindow(window);


#if 0
    if (rd->initialize()) {
        xmit(getEGLDisplay(), getEGLSurface(), getEGLContext(), 0);
        if (onestep) {
            rd->step();
            LOG_E("draw 1");
        } else {
            LOG_E("draw 1 FAILED");
        }
        return;
    }

#else

    static EGLSurface surface;
    static EGLContext context;

    EGLNativeWindowType _window = (EGLNativeWindowType)window;

    const EGLint attribs[] = {
#if ATEST
        EGL_SURFACE_TYPE,   EGL_PBUFFER_BIT,
#else
        // EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        //EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PIXMAP_BIT,
#endif
        EGL_BLUE_SIZE,     8,// 8,
        EGL_GREEN_SIZE,    8,// 8,
        EGL_RED_SIZE,      8,// 8,
        EGL_ALPHA_SIZE,    8,// 8,
        EGL_NATIVE_RENDERABLE, EGL_TRUE,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_BUFFER_SIZE, 32,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,

        EGL_BIND_TO_TEXTURE_RGB, EGL_TRUE,
        EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
#if GLES2
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // 4 ?
#else
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
#endif
        EGL_STENCIL_SIZE, 8,
/*
            EGL_TRANSPARENT_TYPE, EGL_TRANSPARENT_RGB,
            EGL_TRANSPARENT_RED_VALUE,0,
            EGL_TRANSPARENT_BLUE_VALUE,0,
            EGL_TRANSPARENT_GREEN_VALUE,0,*/
            EGL_DEPTH_SIZE, 16,   // <=  if >0 no more see through the terminal window
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    EGLint format;

    //GLfloat ratio;

    ANativeWindow_setBuffersGeometry(_window, 0, 0, format);

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOG_E("eglGetDisplay() returned error %d", eglGetError());
        return;
    }

    EGLint eglMajVers, eglMinVers;


    eglMajVers=0;
    eglMinVers=0;
    if (!eglInitialize(display, &eglMajVers, &eglMinVers)) {
        LOG_E("window.eglInitialize() returned error %d", eglGetError());
        return;
    }

    LOG_I(" >>>>> display pointer set to %p version %d.%d <<<<< ", display, eglMajVers, eglMinVers);


    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        LOG_E("window.eglChooseConfig() returned error %d", eglGetError());
        return;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        LOG_E("window.eglGetConfigAttrib() returned error %d", eglGetError());
        return;
    }

    const EGLint surfaceAttr[] = {
             EGL_WIDTH, 640,
             EGL_HEIGHT, 360,
             EGL_NONE
    };

// EGL_RENDER_BUFFER     EGL_SINGLE_BUFFER,

#if ATEST
    const EGLint ctxAttr[] = {
        //EGL_CONTEXT_CLIENT_VERSION, 2,              // very important!
        EGL_CONTEXT_CLIENT_VERSION, 2,              // very important!
            EGL_NONE
    };


    if ( !(surface = eglCreatePbufferSurface(display, config, surfaceAttr) ) ){
    //if ( !(surface = eglCreateWindowSurface(display, config, _window, surfaceAttr) ) ){
        LOG_I("window.eglCreatePbufferSurface() returned error %d", eglGetError());
        goto fail;
    }

#else
        const EGLint ctxAttr[] = {
#if GLES2
                EGL_CONTEXT_CLIENT_VERSION, 2,              // 2 is N/I for onscreen
#else
                EGL_CONTEXT_CLIENT_VERSION, 1,              // 2 is N/I for onscreen
#endif
                EGL_NONE
        };

        //if ( !(surface = eglCreateWindowSurface(display, config, _window, surfaceAttr) ) ){
        if (!(surface = eglCreateWindowSurface(display, config, _window, 0))) {
            LOG_E("window.eglCreateWindowSurface() returned error %d", eglGetError());
            goto fail;
        }


#endif
        if ( !(context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr)) ){
            LOG_E("window.eglCreateContext() returned error %d", eglGetError());
            goto fail;
        }

        if (!eglMakeCurrent(display, surface, surface, context)) {
            LOG_E("window.eglMakeCurrent() returned error %d", eglGetError());
            goto fail;
        }

/*
    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
        LOG_E("window.eglQuerySurface() returned error %d", eglGetError());
        goto fail;
    }*/

        xmit(display, surface, context, 1);
#endif

        if (onestep) {
            rd->step();
            LOG_E("draw 1");
        } else {
            LOG_E("draw 1 FAILED");
        }
    return;
fail:
        LOG_I("ERROR - Destroying context");
        display = getEGLDisplay();
        if (display){
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            /*
            if (context)
                eglDestroyContext(display, context);
            if (surface)
                eglDestroySurface(display, surface);
            */
            eglTerminate(display);
        }
        return;

    }

    void rd_start() {
        rd->start();
    }

    void rd_step() {
        rd->step();
    }

    void rd_stop(){
        renderingEnabled = false;
    }



}

#endif // RENDERER_H
