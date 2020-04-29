#!/bin/bash

mkdir -p ./prebuilt/wasm

#remove stock emscripten loader
#keep only the polyfill(.js), the cPython interpreter + panda(.wasm) and the filesystem(.data)

cp -uv /data/cross/pydk/projects/org.beerware.pyweb/python.{js,wasm,data} ./prebuilt/wasm/


