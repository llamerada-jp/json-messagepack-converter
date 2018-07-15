cmake_minimum_required(VERSION 2.8)
project(json-messagepack-converter)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/thirdparty/picojson")
include_directories("${PROJECT_SOURCE_DIR}/thirdparty/msgpack-c/include")
link_directories("${PROJECT_SOURCE_DIR}/build/lib")

set("${CMAKE_CXX_FLAGS} -s WASM=1")
set("${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
set("${CMAKE_CXX_FLAGS} -s RESERVED_FUNCTION_POINTERS=1")
set("${CMAKE_CXX_FLAGS} -s EXTRA_EXPORTED_RUNTIME_METHODS=[]")

add_executable(jmc src/jmc.cpp)
target_link_libraries(jmc msgpackc)
