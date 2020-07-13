#define RUBICON 0

#define LIB_PYTHON "lib" PYTHON ".so"

//#define STDLIB ( apk_home "/assets/stdlib.zip")

#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h> // for chdir

#include <locale.h>

#include "Python.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "jni.h"

#include "logger.h"

#define False 0
#define True !False

static bool PY_Initialized = False;

static void *dl=NULL;
static void *py=NULL;

// IO redirector buffer and shared memory
#define IO_BUFSIZE 8192
#define IO_MAX (IO_BUFSIZE * 4)


char cstr[IO_MAX+1];

// asyncio call code
// char aiostr[256];

/*

shared memory construct to avoid need for serialization between processes
    https://bugs.python.org/issue35813

cross compilation of third-party extension modules
    https://bugs.python.org/issue28833

android: add platform.android_ver()
    https://bugs.python.org/issue26855

[ctypes] test_struct_by_value fails on android-24-arm64
    https://bugs.python.org/issue32203

[ctypes] all long double tests fail on android-24-x86_64
    https://bugs.python.org/issue32202

https://bugs.python.org/issue23496
    nice patches from Ryan Gonzalez
    android common pitfalls:
        https://code.google.com/archive/p/python-for-android/wikis/CrossCompilingPython.wiki

https://mail.python.org/pipermail/python-dev/2016-April/144344.html

-----------------------------------------------------------------------
NDK exposes:
    libandroid, liblog, libcamera2ndk, libjnigraphics,
    libmediandk, libOpenMAXAL, libOpenSLES
    libGLES, libvulkan
    libz, libm, libc, libdl, libstdc++

    NOT : sqlite or dalvik

https://blog.quarkslab.com/android-runtime-restrictions-bypass.html
https://stackoverflow.com/questions/44808206/android-jni-call-function-on-android-ui-thread-from-c
https://shipilev.net/blog/2015/black-magic-method-dispatch/
https://rosettacode.org/wiki/Respond_to_an_unknown_method_call
Native Library - Brings Java productivity into C++ program
    https://github.com/tiny-express/native

------------------------------------------------------------------------

Android OpenCV SDK and linking it to the project
    https://github.com/ahasbini/AndroidOpenCVGradlePlugin

Python package for reference counting native pointers
    https://github.com/csiro-hydroinformatics/pyrefcount


Demonstrates how to access and write into the framebuffer directly
    https://github.com/Miouyouyou/Android-framebuffer-Direct-Example


https://android.googlesource.com/platform/bionic/+/master/android-changes-for-ndk-developers.md

force decompression of lib in sdcard to system:
    android.bundle.enableUncompressedNativeLibs=false

Opening shared libraries directly from an APK
In API level 23 and above, it’s possible to open a .so file directly from your APK.
Just use System.loadLibrary("foo") exactly as normal but set android:extractNativeLibs="false"
in your AndroidManifest.xml. In older releases, the .so files were extracted from the APK file at install time.
This meant that they took up space in your APK and again in your installation directory
(and this was counted against you and reported to the user as space taken up by your app).
Any .so file that you want to load directly from your APK must be page aligned (on a 4096-byte boundary)
in the zip file and stored uncompressed. Current versions of the zipalign tool take care of alignment.
Note that in API level 23 and above dlopen(3) will open a library from any zip file, not just your APK.
Just give dlopen(3) a path of the form “my_zip_file.zip!/libs/libstuff.so”.
As with APKs, the library must be page-aligned and stored uncompressed for this to work.


HTTP1.1:
    https://github.com/python-hyper/h11

VT100 FB:
    https://github.com/JulienPalard/vt100-emulator


Events:
    https://www.pythonsheets.com/notes/python-asyncio.html
    https://docs.python.org/fr/3/library/asyncio-sync.html#asyncio.Event
    https://www.programcreek.com/python/example/81578/asyncio.StreamWriter

net:
    https://pfrazee.hashbase.io/blog/hyperswarm
    https://datprotocol.github.io/how-dat-works/
    https://github.com/sp4cerat/Game-NET

dshm:
    https://github.com/sholsapp/gallocy

DUCK - Dalvik Unpythonic Compiler Kit
    https://gitlab.com/dboddie/DUCK/tree/python3

Loading Python modules from code objects dynamically into memory without the use of frozen objects in pure Python.
    https://gist.github.com/cmarshall108/4a6b4922b4998e5d4eef6c87dcb8a88c

Inline asm:
    https://github.com/Maratyszcza/PeachPy
    https://gist.github.com/cmarshall108/01feedf42fd0158f1876c82775367eab
    https://bitbucket.org/pypy/pypy/raw/stm-thread/pypy/doc/stm.rst
    https://docs.micropython.org/en/latest/pyboard/tutorial/assembler.html#pyboard-tutorial-assembler
    https://pypi.org/project/transpyle/
    https://github.com/WAVM/WAVM
    https://github.com/CraneStation/wasmtime
*/

void do_flush_stdout();
void do_stdout_redir();

#include "ioredir.c"


// ================== HELPER "embed" MODULE ===============================
/*
    {"log", mod_embed_log, METH_VARARGS, "Log on android platform"},
    {"cout", mod_embed_cout, METH_VARARGS, "out text to console"},
    {"run", mod_embed_run, METH_VARARGS, "Run on android platform"},
*/


#include "mod_egl.c"

#include "mod_embed.c"


int dir_exists(char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        if (S_ISDIR(st.st_mode))
          return 1;
        }
    return 0;
}

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r") ;
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}


// xxd --include reset  ( xxd from vim )

unsigned char term_reset[] = {
  0x0d, 0x1b, 0x5b, 0x33, 0x67, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x1b, 0x48, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1b,
  0x48, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1b, 0x48, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1b, 0x48, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x1b, 0x48, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x1b, 0x48, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x1b, 0x48, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1b,
  0x48, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1b, 0x48, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1b, 0x48, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x1b, 0x48, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x1b, 0x48, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x1b, 0x48, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1b,
  0x48, 0x0d, 0x1b, 0x63, 0x1b, 0x5b, 0x21, 0x70, 0x1b, 0x5b, 0x3f, 0x33,
  0x3b, 0x34, 0x6c, 0x1b, 0x5b, 0x34, 0x6c, 0x1b, 0x3e, 0x0d, 0x00  // <-- 0x0 end of string !
};


extern PyMODINIT_FUNC PyInit__struct();

JNIEXPORT void JNICALL
Java_{{ cookiecutter.bundle|replace('.', '_') }}_{{ cookiecutter.module_name }}_MainActivity_jnionCreate(JNIEnv *env, jobject instance, jstring jstring_tag,jstring jstring_ver, jstring jstring_apk, jstring jstring_lib, jstring jstring_home) {
    if (!PY_Initialized) {
        LOG_W(" ============== onCreate : C Begin ================");

        // set global apk logging tag
        LOG_TAG = (*env)->GetStringUTFChars( env, jstring_tag, NULL ) ;
#if 1
        // log a full term reset, to get cursor home and see clean app startup
        // also empty terminal app buffers could be usefull for long session with unlimited scrollback on.
        if ( sizeof(cstr) > sizeof(term_reset))
            memcpy(&cstr[0],&term_reset[0], sizeof(term_reset));

        LOG(LOG_TAG,cstr);
#endif
        // python version lib name, to use directly python3.? folders found in prefix/lib
        const char *python  =  (*env)->GetStringUTFChars( env, jstring_ver , NULL ) ;
        // stdlib archive path (apk==zip)
        const char *apk_path = (*env)->GetStringUTFChars( env, jstring_apk , NULL ) ;
        // workd directory will chdir here.
        const char *apk_home = (*env)->GetStringUTFChars( env, jstring_home, NULL ) ;

        const char *apk_lib =  (*env)->GetStringUTFChars( env, jstring_lib, NULL ) ;

        // first go somewhere writeable !
        chdir(apk_home);

        LOG_W("stdlib(inside APK) :");
        LOG(LOG_TAG, apk_path );
        LOG_W("HOME(apk directory) :");
        LOG(LOG_TAG, apk_home );

#if RUBICON
        // set global for rubicon.
        java = env;
#endif
        if (!mkdir("dev", 0700)) {
           LOG_V("no 'dev' directory, creating one ...");
        }

        snprintf(stdout_redir_fn, sizeof(stdout_redir_fn), "%s/dev/stdout", apk_home );

        if (stdout_redir_enabled)
            do_stdout_redir();

        if (!mkdir("tmp", 0700)) {
           LOG_V("no 'tmp' directory, creating one ...");
        }

        snprintf(cstr, sizeof(cstr), "%s/tmp", apk_home );
        setenv("TEMP", cstr, 0);
        setenv("TMP", cstr, 0);

        setenv("XDG_CONFIG_HOME", apk_home, 1);
        setenv("XDG_CACHE_HOME", apk_home, 1);

        // be a bit more nix friendly
        setenv("HOME", apk_home, 1);

        setenv("DYLD", apk_lib, 1 );

        // potentially all apps signed from a same editor could have same UID  ( shared-uid )
        // though different apk names.
        setenv("USER", LOG_TAG, 1);

        snprintf(cstr, sizeof(cstr), "%s", apk_home );
        char* token = strtok(cstr, "/");
        while (token != NULL) {
            setenv("USERNAME", token, 1);
            token = strtok(NULL, "/");
        }

        setenv("PYTHONOPTIMIZE","No",1);

        setenv("PYTHONDONTWRITEBYTECODE","1",1);

        snprintf(cstr, sizeof(cstr), "%s/usr", apk_home );
        setenv("PYTHONHOME", cstr, 1);

        setenv("PYTHONCOERCECLOCALE","1",1);
        setenv("PYTHONUNBUFFERED","1",1);

        // TODO: pip binary modules
        // TODO: PYTHONPYCACHEPREFIX
        //setenv("PYTHONHOME", apk_home + "/usr", 1);

        //dlopen("dl", RTLD_NOW);
        //dlopen(LIB_PYTHON, RTLD_NOW);

        Py_SetProgramName((const wchar_t *)LOG_TAG);

        setlocale(LC_ALL, "C.UTF-8");

        // add our support module
        PyImport_AppendInittab("embed", init_embed);

        snprintf(cstr, sizeof(cstr), "%s/assets", apk_home );
        if (dir_exists(cstr)) {
            snprintf(cstr, sizeof(cstr), "%s/assets/%s", apk_home, python );
            if (dir_exists(cstr)) {
                // test mode use plain files for everything
                LOG_W("!!!!!!!! TESTSUITE MODE !!!!!!!!!!!!");
                snprintf(cstr, sizeof(cstr), "%s/assets/%s:%s/assets", apk_home, python, apk_home );
            } else {
                // dev mode use plain files for not stdlib, and comes first
                LOG_W(" !!!!!!!!!!! DEV MODE !!!!!!!!!!!!");
                //snprintf(cstr, sizeof(cstr), "%s/lib:%s/assets:%s/assets/%s", apk_home, apk_home, apk_path, python);
                snprintf(cstr, sizeof(cstr), "%s/assets:%s/assets:%s/assets/%s:%s/lib", apk_home, apk_path, apk_path, python, apk_home);
            }
        } else
            snprintf(cstr, sizeof(cstr), "%s/assets/%s:%s/assets", apk_path, python, apk_path);


        LOG_V("Setting paths ... ");
        LOG(LOG_TAG, cstr);
        setenv("PYTHONPATH", cstr, 1);

//P1

    }

    LOG_W(" ============== onCreate : C end ================");
}

void Python_stop(){
    fclose(stdout_redir);
    fclose(stdout);
}


#include "wasm3.h"
#include "m3_env.h"
#include "m3_config.h"
#include "m3_exception.h"

//extern unsigned int fib32_wasm_len;
//extern unsigned char fib32_wasm[];

#define WAPY (1)


#if WAPY
    #include "wapy.h"
    #define WASM_DATA upy_wasi
    #define WASM_LEN upy_wasi_len
    #define ENTRY_POINT "main"
    const char* i_argv[] = {};
    uint32_t i_argc = 0;


#else
    #include "extra/fib32.wasm.h"
    #define ENTRY_POINT "fib"
    #define WASM_DATA fib32_wasm
    #define WASM_LEN fib32_wasm_len
    const char* i_argv[2] = { "20", NULL };
    uint32_t i_argc = 1;
#endif


#define FATAL(msg, ...) { printf("Fatal: " msg "\n", ##__VA_ARGS__); return; }

struct VM3 {
    IM3Environment env ;
    IM3Module module;
    IM3Runtime runtime;
    IM3Function i_entrypoint;
    M3Result result;
};

IM3Function vm_wasm_init(struct VM3 * vm, uint8_t* wasm, size_t fsize, const char *entrypoint, uint32_t argc, const char* argv[]) {
    LOG_W("Loading WebAssembly...");
    M3Result result = m3Err_none;

    vm->i_entrypoint = NULL;
    vm->result = m3Err_none;
    vm->env = m3_NewEnvironment();
    if (!(vm->env)) LOG_W("m3_NewEnvironment failed");

    vm->runtime = m3_NewRuntime (vm->env, 32768, NULL);
    if (!vm->runtime) LOG_W("m3_NewRuntime failed");

    result = m3_ParseModule (vm->env, &(vm->module), wasm, fsize);
    if (result) LOG_W("m3_ParseModule != 0");

    result = m3_LoadModule (vm->runtime, vm->module);
    if (result) LOG_W("m3_LoadModule != 0");

    result = m3_FindFunction (&(vm->i_entrypoint), vm->runtime, entrypoint);
    if (result) LOG_W("m3_FindFunction(%s):  %s", entrypoint, result);

    vm->runtime->argc = argc;
    vm->runtime->argv = argv;

    return vm->i_entrypoint;
}




#define VMX vmslots[ctx]
void vm_wasm_run(int ctx)
{
    static IM3Function i_function = NULL;

    static struct VM3 vmslots[1];


    M3Result result;


    if (!i_function) {
        i_function = vm_wasm_init(&VMX, (uint8_t*)WASM_DATA, WASM_LEN-1, ENTRY_POINT, i_argc, i_argv);


        //IM3Environment env = VMX.env;

        if(!i_function) {
            LOG_W("failed to locate entry point in wasm module");
            return;
        }


        if (!i_function->compiled) {
            LOG_W("wasm code is not compiled");
            _throw(m3Err_missingCompiledCode);
            goto _catch;
        }

        LOG_W("First pass ...");
        M3Result result = m3Err_none;


        if (i_function->name and strcmp(i_function->name, "_start") == 0)       // WASI
            i_argc = 0;

        IM3FuncType ftype = i_function->funcType;
        m3log(VMX.runtime, "calling %s", SPrintFuncTypeSignature(ftype));

        if (i_argc != ftype->numArgs)
            _throw(m3Err_argumentCountMismatch);

        // args are always 64-bit aligned
        u64 *stack = (u64 *) VMX.runtime->stack;

        // The format is currently not user-friendly by default,
        // as this is used in spec tests
        for (u32 i = 0; i < ftype->numArgs; ++i) {
            u64 *s = &stack[i];
            ccstr_t str = i_argv[i];
            switch (ftype->argTypes[i]) {
                case c_m3Type_i32:
                case c_m3Type_f32:
                    *(u32 *) (s) = strtoul(str, NULL, 10);
                    break;
                case c_m3Type_i64:
                case c_m3Type_f64:
                    *(u64 *) (s) = strtoull(str, NULL, 10);
                    break;
                default:
                    _throw("unknown argument type");
            }
        }
        m3StackCheckInit();
    }

    if (i_function) {
        LOG_W("Running ...");
        u64 *stack = (u64 *) VMX.runtime->stack;
        IM3FuncType ftype = i_function->funcType;

        _((M3Result) Call(i_function->compiled, (m3stack_t) stack, VMX.runtime->memory.mallocated, d_m3OpDefaultArgs));

        #if d_m3LogOutput
            switch (ftype->returnType) {

                case c_m3Type_none:
                    fprintf(stderr, "Result: <Empty Stack>\n");
                    break;

                #ifdef USE_HUMAN_FRIENDLY_ARGS
                case c_m3Type_i32:
                    fprintf(stderr, "Result: %" PRIi32 "\n", *(i32 *) (stack));
                    break;
                case c_m3Type_i64:
                    fprintf(stderr, "Result: %" PRIi64 "\n", *(i64 *) (stack));
                    break;
                case c_m3Type_f32:
                    fprintf(stderr, "Result: %f\n", *(f32 *) (stack));
                    break;
                case c_m3Type_f64:
                    fprintf(stderr, "Result: %lf\n", *(f64 *) (stack));
                    break;
                #else
                case c_m3Type_i32:
                case c_m3Type_f32:
                    fprintf(stderr, "Result: %u\n", *(u32 *) (stack));
                    break;
                case c_m3Type_i64:
                case c_m3Type_f64:
                    fprintf(stderr, "Result: %" PRIu64 "\n", *(u64 *) (stack));
                    break;
                #endif // USE_HUMAN_FRIENDLY_ARGS

                default:
                    _throw("unknown return type");
            }

        #if d_m3LogNativeStack
            size_t stackUsed = m3StackGetMax();
            fprintf(stderr, "Native stack used: %d\n", stackUsed);
        #endif // d_m3LogNativeStack

        #endif // d_m3LogOutput


    }
_catch: {

    long value = *(uint64_t*)(VMX.runtime->stack);
    LOG_W("Result-exit: %ld\n", value);
}

}
#undef VMX

JNIEXPORT jstring JNICALL
Java_{{ cookiecutter.bundle|replace('.', '_') }}_{{ cookiecutter.module_name }}_MainActivity_PyRun(JNIEnv *env, jobject instance, jstring jstring_code) {
    if (stdout_redir_enabled)
        do_stdout_redir();
    const char *code = (*env)->GetStringUTFChars( env, jstring_code , NULL );
    PyRun_SimpleString(code);
    (*env)->ReleaseStringUTFChars(env,jstring_code, code);
    return (*env)->NewStringUTF(env, cstr);
}

/*
 * Main working thread function. From a pthread,
 *     calling back to MainActivity::updateTimer() to display ticks on UI
 *     calling back to JniHelper::updateStatus(String msg) for msg
 */
void*
VMthread(void* context) {



    TickContext *pctx = (TickContext*) context;
    JavaVM *javaVM = pctx->javaVM;
    JNIEnv *env;

    jint res = (*javaVM)->GetEnv(javaVM, (void**)&env, JNI_VERSION_1_6);

    if (res != JNI_OK) {
        res = (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);
        if (JNI_OK != res) {
            LOG_E("Failed to AttachCurrentThread, ErrorCode = %d", res);
            return NULL;
        }
    }

    LOG_I("VMthread starting");
//--------------------------------------------
//P1
    cstr[IO_MAX] = 0;

        LOG_V("Initializing Python... ");
        //vm_wasm_run(0);

        Py_Initialize();
        LOG_V("/Initializing Python ... ");

        /* ensure threads will work. */
        LOG_V("Initializing Python threads ...");
        PyEval_InitThreads();
        LOG_V("/Initializing Python threads ...");

        PY_Initialized = 1;

        //PyRun_SimpleString("import pythons");

        do_flush_stdout();

        PyRun_SimpleString("print('42 '*42);print()");

        //PyRun_SimpleString("import utime;print( (' [%s] ' % int(time.time()))*42);print()");
        //PyRun_SimpleString("import sys;print( (' [%r] ' % sys)*42);print()");


//--------------------------------------------

    jmethodID statusId = (*env)->GetMethodID(env, pctx->jniHelperClz, "updateStatus", "(Ljava/lang/String;)V");
    sendJavaMsg(env, pctx->jniHelperObj, statusId, "TickerThread status: initializing...");

    // get mainActivity updateTimer function
    jmethodID timerId = (*env)->GetMethodID(env, pctx->mainActivityClz, "updateTimer", "()V");

    struct timeval beginTime, curTime, usedTime, leftTime;

    // ~ 60 fps / 2
    const struct timeval kOneFrame = {
            (__kernel_time_t)0,
            (__kernel_suseconds_t) 32000
    };

    sendJavaMsg(env, pctx->jniHelperObj, statusId, "TickerThread status: start ticking ...");


    //vm_wasm_run(0);


    static int init_egl_done = 0;



    while(1) {

        static unsigned long steps = 0;

        gettimeofday(&beginTime, NULL);

        //LOG_V("VMthread lock acquire");
        pthread_mutex_lock(&pctx->lock);

        int done = pctx->done;

        if (pctx->done) {
            pctx->done = 0;
        }

        do_flush_stdout();

        pthread_mutex_unlock(&pctx->lock);
        //LOG_V("VMthread lock release");

        if (done) {
            LOG_V("VMthread exiting");
            break;
        }

        if (window) {
            if (!init_egl_done) {
                rd_init( window, 1);
                //xmit(getEGLDisplay(), getEGLSurface(), getEGLContext(), 0);
                init_egl_done=1;
                //rd_step();
            }
            //Java_{{ cookiecutter.bundle|replace('.', '_') }}_{{ cookiecutter.module_name }}_MainActivity_PyLoop
        }

        PyRun_SimpleString("print(1)");
        //PyRun_SimpleString("python3.on_step(Applications,python3)");

        //(*env)->CallVoidMethod(env, pctx->mainActivityObj, timerId);



        gettimeofday(&curTime, NULL);
        timersub(&curTime, &beginTime, &usedTime);
        timersub(&kOneFrame, &usedTime, &leftTime);
        struct timespec sleepTime;
        sleepTime.tv_sec = leftTime.tv_sec;
        sleepTime.tv_nsec = leftTime.tv_usec * 1000;

        if (sleepTime.tv_nsec <= 32000000) {
            nanosleep(&sleepTime, NULL);
            //LOG_V("ok");
        } else {
            LOG_V("VMthread is late");
//TODO: every 10 seconds announce count late frames
            //sendJavaMsg(env, pctx->jniHelperObj, statusId, "TickerThread error: processing too long!");
        }

    }

    LOG_V("VMthread exited");

    sendJavaMsg(env, pctx->jniHelperObj, statusId, "TickerThread status: ticking stopped");
    (*javaVM)->DetachCurrentThread(javaVM);
    return context;
}

