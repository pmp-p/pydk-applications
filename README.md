# pydk-applications

WIP <= i mean it

This project may also automatically embed https://github.com/wasm3/wasm3


Tools and samples for building mobile/web apps with Python

This will cover only the web part, android prebuilts are huge (and quite unsafe) : build them yourself with PyDK.
or use the libwapy.so prebuilt, note that you'll need to link your "android-sdk" folder into the minimalist ./pydk/


~~Here is offered a webassembly loader, which is a CPython interpreter, a big part of CPython stdlib and also Panda3D modules
Since not everybody runs a webserver, a small Python3.7+ utility is provided to let you test you run apps.
It shows a web menu to let you pick up samples and try them.~~ currently reworked.


example (with device):

PYDK_ABI=wapy PYDK=$(pwd)/pydk ./build.sh org.beerware.wapy

example (without device):

ANDROID_HOME=/home/someone/studio/android-sdk DEVICE=false PYDK_ABI=wapy PYDK=$(pwd)/pydk ./build.sh org.beerware.wapy


available ABI:

wapy ( wapy+ffi or pycopy )
aosp-38 ( cpython 3.8.x + Panda3D)

wasm support will be implicitly added but browser loader is not written yet.


Provided pydk is prebuilt minimal - only wapy support - to get other choices build the full sdk.




