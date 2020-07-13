# https://boringssl.googlesource.com/boringssl/+/version_for_cocoapods_6.0/third_party/android-cmake/README.md

# -DANDROID_ABI="armeabi-v7a with NEON" ?


project(app)
cmake_minimum_required(VERSION 3.4.1)


set(ANDROID_STL "c++_shared")

set(ROOT ${CMAKE_SOURCE_DIR}/../../../..)

set(ASSETS ${ROOT}/assets)

set(PYDK "${ROOT}/../pydk/")


set(TARGET rmipython)

set(APKUSR "${PYDK}/aosp/apkroot-${ANDROID_NDK_ABI_NAME}/usr")

set(PREBUILT "${ROOT}/prebuilt/${ANDROID_NDK_ABI_NAME}")

function(ld name libfile)
    add_library(${name} SHARED IMPORTED )
    set_target_properties(${name} PROPERTIES IMPORTED_LOCATION ${PREBUILT}/${libfile})
    set_target_properties(${name} PROPERTIES IMPORTED_NO_SONAME ON)
    execute_process(COMMAND sh -c "mkdir -p ${PREBUILT};/bin/cp -vu ${APKUSR}/lib/${libfile} ${PREBUILT}/")
    execute_process(COMMAND sh -c "mkdir -p ${PREBUILT};/bin/cp -vu ${REPO}/${libfile} ${PREBUILT}/")

endfunction(ld)




if (CPYTHON)
    message(STATUS "     ============= cpython ${CPYTHON} =============")
    set(REPO "${PYDK}/prebuilt/${ANDROID_NDK_ABI_NAME}/${CPYTHON}")

    set(PYVER 3.8)
    set(PYTHON python${PYVER})

    set(PREINC "${PYDK}/aosp/apkroot-${ANDROID_NDK_ABI_NAME}/usr/include")

    include_directories(${PREINC} ${PREINC}/${PYTHON} ${CMAKE_SOURCE_DIR}/Include)

    ld(sslpython "libsslpython.so")

    ld(cryptopython "libcryptopython.so")

    ld(lzma "liblzma.so")

    ld(BZ2 "libbz2.so")

    ld(ffi "libffi.so")

    ld(brokenthings "libbrokenthings.so")

    add_library(python3 SHARED IMPORTED )
    set_target_properties(python3 PROPERTIES IMPORTED_LOCATION ${PREBUILT}/lib${PYTHON}.so)
    set_target_properties(python3 PROPERTIES IMPORTED_NO_SONAME ON)

    # Include libraries needed for rmipython lib
    set(TARGET ${TARGET}
        lzma
        BZ2
        ffi
        ssl
        crypto
        brokenthings
        python3
    )
#include(${BUILT}.include)
#                     -Wl,-whole-archive
#                     ${PREBUILT}/lib/libpython3.7.a
#                      -Wl,--no-whole-archive)
endif()

if (WAPY)

    set(REPO "${PYDK}/prebuilt/${ANDROID_NDK_ABI_NAME}/${WAPY}")


    message(WARNING " >>>> PYTHON=${PYDK}/prebuilt/${ANDROID_NDK_ABI_NAME}/${WAPY}")

    include_directories("${PYDK}/prebuilt/${ANDROID_NDK_ABI_NAME}/${WAPY}")

    set(WASM3_PATH "${ROOT}/../wasm3/source")

    message(STATUS " >>>> WASM3_PATH=${WASM3_PATH}")

    file(GLOB WASM3_SRC "${WASM3_PATH}/*.c")

    message(WARNING " >>>> WASM3_SRC=${WASM3_SRC}")

    add_library(wasm3-jni SHARED ${WASM3_SRC})
    include_directories(${WASM3_PATH})

    #add_library(dl SHARED "libdl.so")


    ld(ffi "libffi.so")

    ld(python3 "lib${WAPY}.so")

    set(TARGET ${TARGET}
        wasm3-jni
        python3
        dl
    )
endif()



# copy arch dependant libs
#execute_process(COMMAND sh -c "mkdir -p ${PREBUILT};/bin/cp -vu ${REPO}/*.so ${PREBUILT}/")
#execute_process(COMMAND sh -c "mkdir -p ${PREBUILT};/bin/cp ${REPO}-extra/*.so ${PREBUILT}/")
# copy sysconfig data
#execute_process(COMMAND sh -c "/bin/cp ${ASSETS}/python${PYVER}/_sysconfig*.py ${ASSETS}/python${PYVER}/")


set(TARGET ${TARGET}
        c++_shared
        GLESv2
        GLESv1_CM
        EGL
        android
        log
        z
)

add_library(rmipython SHARED rmipython.c mod_egl.cpp)
target_link_libraries(${TARGET})









