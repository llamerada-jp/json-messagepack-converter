#!/bin/env sh

set -eu

cd `dirname $0`
readonly ROOT_PATH="`pwd`"
readonly BUILD_PATH=${ROOT_PATH}/build

# msgpack for C/C++
mkdir -p ${BUILD_PATH}/msgpack
cd ${BUILD_PATH}/msgpack
emcmake cmake -DMSGPACK_CXX11=ON -DMSGPACK_BUILD_TESTS=OFF -DMSGPACK_BUILD_EXAMPLES=OFF -DCMAKE_CXX_FLAGS="-s WASM=1" -DCMAKE_C_FLAGS="-s WASM=1" -DCMAKE_INSTALL_PREFIX=${BUILD_PATH} ${ROOT_PATH}/thirdparty/msgpack-c
make install

mkdir -p ${BUILD_PATH}/jmc
cd ${BUILD_PATH}/jmc
emcmake cmake ${ROOT_PATH}
make -j
